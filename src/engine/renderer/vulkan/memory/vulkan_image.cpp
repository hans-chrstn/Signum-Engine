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
#include <string>

namespace SNE::Engine::Renderer::Vulkan {
    VulkanImage::VulkanImage(const VulkanMemoryAllocator &allocator,
                             const VulkanImageCreateInfo &create_info)
        : m_Allocator(allocator.nativeHandle()) {
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

        VkImageCreateInfo image_create_info{};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.imageType = create_info.image_type;
        image_create_info.extent = create_info.extent;
        image_create_info.mipLevels = 1U;
        image_create_info.arrayLayers = 1U;
        image_create_info.format = create_info.format;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_create_info.usage = create_info.usage;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        const VulkanMemoryPolicy memory_policy =
            selectVulkanMemoryPolicy(GpuMemoryUsage::Device);

        VmaAllocationCreateInfo allocation_create_info{};
        allocation_create_info.usage = memory_policy.usage;
        allocation_create_info.flags = memory_policy.flags;

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

    VulkanImage::~VulkanImage() noexcept {
        if (m_Image != VK_NULL_HANDLE && m_Allocation != nullptr) {
            vmaDestroyImage(m_Allocator, m_Image, m_Allocation);
        }

        m_Image = VK_NULL_HANDLE;
        m_Allocation = nullptr;
        m_Allocator = nullptr;
    }
} // namespace SNE::Engine::Renderer::Vulkan
