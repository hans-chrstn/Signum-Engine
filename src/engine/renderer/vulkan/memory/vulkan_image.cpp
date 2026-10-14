#include "vulkan_image.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/gpu_memory_usage.hpp"
#include "engine/renderer/vulkan/common/vulkan_result.hpp"
#include "vulkan_memory_policy.hpp"
#include <algorithm>
#include <bit>
#include <string>
#include <utility>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;
namespace Core = SNE::Engine::Core;

namespace {
    auto
    validateImageDimensions(const Vulkan::VulkanImageCreateInfo &create_info)
        -> void {
        switch (create_info.image_type) {
        case VK_IMAGE_TYPE_1D:
            if (create_info.extent.width == 0U) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanImage requires a non-zero width for 1D images");
            }

            if (create_info.extent.height != 1U) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanImage requires a height of one for 1D images");
            }

            if (create_info.extent.depth != 1U) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanImage requires a depth of one for 1D images");
            }
            break;
        case VK_IMAGE_TYPE_2D:
            if (create_info.extent.width == 0U ||
                create_info.extent.height == 0U) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanImage requires a non-zero width and height for 2D "
                    "images");
            }

            if (create_info.extent.depth != 1U) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanImage requires a depth of one for 2D images");
            }
            break;
        case VK_IMAGE_TYPE_3D:
            if (create_info.extent.width == 0U ||
                create_info.extent.height == 0U ||
                create_info.extent.depth == 0U) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanImage requires a non-zero extent for 3D images");
            }
            break;
        default:
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanImage requires a supported image type");
        }
    }

    auto
    validateImageCreateInfo(const Vulkan::VulkanImageCreateInfo &create_info)
        -> void {

        if (create_info.format == VK_FORMAT_UNDEFINED) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanImage requires a defined image format");
        }

        if (create_info.usage == 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanImage requires at least one image usage flag");
        }
    }

    auto
    validateImageMipLevels(const Vulkan::VulkanImageCreateInfo &create_info)
        -> void {
        if (create_info.mip_levels == 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanImage requires at least one mip level");
        }

        const std::uint32_t largest_dimension = std::max({
            create_info.extent.width,
            create_info.extent.height,
            create_info.extent.depth,
        });

        const auto max_mip_levels =
            static_cast<std::uint32_t>(std::bit_width(largest_dimension));

        if (create_info.mip_levels > max_mip_levels) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanImage mip level count exceeds the image dimensions");
        }
    }

    auto validateImageLayerAndSampleRules(
        const Vulkan::VulkanImageCreateInfo &create_info) -> void {
        const bool is_multisampled =
            create_info.samples != VK_SAMPLE_COUNT_1_BIT;

        const bool cube_compatible =
            (create_info.flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) != 0U;

        if (create_info.array_layers == 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanImage requires at least one array layer");
        }

        if (create_info.image_type == VK_IMAGE_TYPE_3D &&
            create_info.array_layers != 1U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanImage requires exactly one array layer for 3D images");
        }

        if (is_multisampled && create_info.image_type != VK_IMAGE_TYPE_2D) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanImage multisampled images must be 2D");
        }

        if (is_multisampled && create_info.mip_levels != 1U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanImage multisampled images require exactly one mip "
                "level");
        }

        if (is_multisampled && cube_compatible) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanImage multisampled images cannot be cube-compatible");
        }

        constexpr std::uint32_t kCubeFaceCount = 6U;

        if (cube_compatible) {
            if (create_info.image_type != VK_IMAGE_TYPE_2D) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanImage cube-compatible images must be 2D");
            }
            if (create_info.extent.width != create_info.extent.height) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanImage cube-compatible images require equal width "
                    "and height");
            }
            if (create_info.array_layers < kCubeFaceCount) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanImage cube-compatible images require at least six "
                    "array layers");
            }
        }
    }
} // namespace

namespace SNE::Engine::Renderer::Vulkan {
    VulkanImage::VulkanImage(const VulkanMemoryAllocator &allocator,
                             const VulkanImageCreateInfo &create_info)
        : m_Allocator(allocator.nativeHandle()), m_CreateInfo(create_info) {
        validateImageDimensions(create_info);
        validateImageCreateInfo(create_info);
        validateImageMipLevels(create_info);
        validateImageLayerAndSampleRules(create_info);

        VkImageCreateInfo image_create_info{};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.imageType = create_info.image_type;
        image_create_info.extent = create_info.extent;
        image_create_info.mipLevels = create_info.mip_levels;
        image_create_info.arrayLayers = create_info.array_layers;
        image_create_info.format = create_info.format;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_create_info.usage = create_info.usage;
        image_create_info.samples = create_info.samples;
        image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_create_info.flags = create_info.flags;

        const VulkanMemoryPolicy memory_policy =
            selectVulkanMemoryPolicy(GpuMemoryUsage::Device);

        VmaAllocationCreateInfo allocation_create_info{};
        allocation_create_info.usage = memory_policy.usage;
        allocation_create_info.flags = memory_policy.flags;

        VkImageFormatProperties image_format_properties{};

        const VkResult image_format_properties_result =
            vkGetPhysicalDeviceImageFormatProperties(
                allocator.physicalDevice(), create_info.format,
                create_info.image_type, VK_IMAGE_TILING_OPTIMAL,
                create_info.usage, image_create_info.flags,
                &image_format_properties);

        if (image_format_properties_result == VK_ERROR_FORMAT_NOT_SUPPORTED) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanImageFormatUnsupported,
                "Requested Vulkan image format configuration is unsupported",
                Core::Error::NativeError(
                    static_cast<int>(image_format_properties_result),
                    std::string(toString(image_format_properties_result))),
                "Query Vulkan Image Format Support");
        }

        if (image_format_properties_result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanImageFormatQueryFailed,
                "Failed to query Vulkan image format capabilities",
                Core::Error::NativeError(
                    static_cast<int>(image_format_properties_result),
                    std::string(toString(image_format_properties_result))),
                "Query Vulkan Image Format Capabilities");
        }

        if (create_info.mip_levels > image_format_properties.maxMipLevels) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanImageFormatUnsupported,
                "Requested Vulkan image mip level count exceeds device "
                "capabilities",
                "Validate Vulkan Image Mip Level Support");
        }

        if (create_info.array_layers > image_format_properties.maxArrayLayers) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanImageFormatUnsupported,
                "Requested Vulkan image array layer count exceeds device "
                "capabilities",
                "Validate Vulkan Image Array Layer Support");
        }

        if (create_info.extent.width >
                image_format_properties.maxExtent.width ||
            create_info.extent.height >
                image_format_properties.maxExtent.height ||
            create_info.extent.depth >
                image_format_properties.maxExtent.depth) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanImageFormatUnsupported,
                "Requested Vulkan image extent exceeds device capabilities",
                "Validate Vulkan Image Extent Support");
        }

        if ((image_format_properties.sampleCounts &
             static_cast<VkSampleCountFlags>(image_create_info.samples)) ==
            0U) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanImageFormatUnsupported,
                "Requested Vulkan image sample count exceeds device "
                "capabilities",
                "Validate Vulkan Image Sample Count Support");
        }

        const VkResult result = vmaCreateImage(
            m_Allocator, &image_create_info, &allocation_create_info, &m_Image,
            &m_Allocation, nullptr);

        if (result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanImageCreationFailed,
                "Failed to create Vulkan image",
                Core::Error::NativeError(static_cast<int>(result),
                                         std::string(toString(result))),
                "Create Vulkan Image");
        }

        if (m_Image == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Postcondition,
                Core::Error::Subsystem::Vulkan,
                "Successful VMA image creation must produce a non-null Vulkan "
                "image handle");
        }

        if (m_Allocation == nullptr) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Postcondition,
                Core::Error::Subsystem::Vulkan,
                "Successful VMA image creation must produce a valid VMA "
                "allocation");
        }
    }

    auto VulkanImage::destroy() noexcept -> void {
        if (m_Image != VK_NULL_HANDLE && m_Allocation != nullptr) {
            vmaDestroyImage(m_Allocator, m_Image, m_Allocation);
        }

        m_Image = VK_NULL_HANDLE;
        m_Allocation = nullptr;
        m_Allocator = nullptr;
        m_CreateInfo = VulkanImageCreateInfo{};
    }

    VulkanImage::~VulkanImage() noexcept {
        destroy();
    }

    VulkanImage::VulkanImage(VulkanImage &&other) noexcept
        : m_Allocator(std::exchange(other.m_Allocator, nullptr)),
          m_Image(std::exchange(other.m_Image, VK_NULL_HANDLE)),
          m_Allocation(std::exchange(other.m_Allocation, nullptr)),
          m_CreateInfo(
              std::exchange(other.m_CreateInfo, VulkanImageCreateInfo{})) {}

    auto VulkanImage::operator=(VulkanImage &&other) noexcept -> VulkanImage & {
        if (this == &other) {
            return *this;
        }

        destroy();

        m_Allocator = std::exchange(other.m_Allocator, nullptr);
        m_Image = std::exchange(other.m_Image, VK_NULL_HANDLE);
        m_Allocation = std::exchange(other.m_Allocation, nullptr);
        m_CreateInfo =
            std::exchange(other.m_CreateInfo, VulkanImageCreateInfo{});

        return *this;
    }

    auto VulkanImage::nativeHandle() const noexcept -> VkImage {
        return m_Image;
    }

    auto VulkanImage::usage() const noexcept -> VkImageUsageFlags {
        return m_CreateInfo.usage;
    }

    auto VulkanImage::createInfo() const noexcept
        -> const VulkanImageCreateInfo & {
        return m_CreateInfo;
    }
} // namespace SNE::Engine::Renderer::Vulkan
