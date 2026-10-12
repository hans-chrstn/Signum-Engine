#include "vulkan_image_view.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/vulkan/common/vulkan_result.hpp"
#include <string>
#include <utility>

namespace SNE::Engine::Renderer::Vulkan {
    VulkanImageView::VulkanImageView(VkDevice device,
                                     const VkImageViewCreateInfo &create_info)
        : m_Device(device) {
        if (device == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanImageView requires a valid Vulkan device");
        }

        if (create_info.image == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanImageView requires a valid Vulkan image");
        }

        if (create_info.format == VK_FORMAT_UNDEFINED) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanImageView requires a defined image format");
        }

        if (create_info.sType != VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanImageView requires a valid image-view creation "
                "structure type");
        }

        if (create_info.subresourceRange.aspectMask == 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanImageView requires at least one image aspect");
        }

        const VkResult result =
            vkCreateImageView(m_Device, &create_info, nullptr, &m_ImageView);

        if (result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanImageViewCreationFailed,
                "Failed to create Vulkan image view",
                Core::Error::NativeError(static_cast<int>(result),
                                         std::string(toString(result))),
                "Create Vulkan Image View");
        }

        if (m_ImageView == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Postcondition,
                Core::Error::Subsystem::Vulkan,
                "Successful Vulkan image-view creation must produce a non-null "
                "image view handle");
        }
    }

    auto VulkanImageView::destroy() noexcept -> void {
        if (m_ImageView != VK_NULL_HANDLE) {
            vkDestroyImageView(m_Device, m_ImageView, nullptr);
        }

        m_Device = VK_NULL_HANDLE;
        m_ImageView = VK_NULL_HANDLE;
    }

    VulkanImageView::~VulkanImageView() noexcept {
        destroy();
    }

    VulkanImageView::VulkanImageView(VulkanImageView &&other) noexcept
        : m_Device(std::exchange(other.m_Device, VK_NULL_HANDLE)),
          m_ImageView(std::exchange(other.m_ImageView, VK_NULL_HANDLE)) {}

    auto VulkanImageView::operator=(VulkanImageView &&other) noexcept
        -> VulkanImageView & {
        if (this == &other) {
            return *this;
        }

        destroy();

        m_Device = std::exchange(other.m_Device, VK_NULL_HANDLE);
        m_ImageView = std::exchange(other.m_ImageView, VK_NULL_HANDLE);

        return *this;
    }

    auto VulkanImageView::nativeHandle() const noexcept -> VkImageView {
        return m_ImageView;
    }
} // namespace SNE::Engine::Renderer::Vulkan
