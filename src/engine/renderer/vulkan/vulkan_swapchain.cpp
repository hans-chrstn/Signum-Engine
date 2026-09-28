#include "vulkan_swapchain.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "vulkan_result.hpp"
#include "vulkan_swapchain_selection.hpp"
#include "vulkan_swapchain_support.hpp"
#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace SNE::Engine::Renderer::Vulkan {
    VulkanSwapchain::VulkanSwapchain(
        VkPhysicalDevice physical_device, VkDevice logical_device,
        VkSurfaceKHR surface, const QueueFamilyIndices &queue_family_indices,
        const Platform::FramebufferSize &framebuffer_size,
        PresentationPreference presentation_preference,
        bool fifo_latest_ready_enabled)
        : m_Device(logical_device) {
        const SwapchainSupportDetails swapchain_support =
            querySwapchainSupport(physical_device, surface);

        m_SurfaceFormat =
            selectSurfaceFormat(swapchain_support.available_surface_formats);

        const VkPresentModeKHR presentation_mode = selectPresentationMode(
            presentation_preference,
            swapchain_support.available_presentation_modes,
            fifo_latest_ready_enabled);

        m_Extent = selectSwapExtent(swapchain_support.surface_capabilities,
                                    framebuffer_size);

        const std::uint32_t selected_swapchain_image_count =
            selectSwapchainImageCount(swapchain_support.surface_capabilities);

        if (!queue_family_indices.graphics_family.has_value() ||
            !queue_family_indices.presentation_family.has_value()) {
            throw std::logic_error("VulkanSwapchain requires graphics and "
                                   "presentation queue families");
        }

        const std::uint32_t queue_graphics_index =
            queue_family_indices.graphics_family.value();
        const std::uint32_t queue_presentation_index =
            queue_family_indices.presentation_family.value();

        const std::array<std::uint32_t, 2> queue_family{
            {queue_graphics_index, queue_presentation_index}};

        VkSwapchainCreateInfoKHR swapchain_create_info{};
        swapchain_create_info.sType =
            VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

        if (queue_graphics_index != queue_presentation_index) {
            swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchain_create_info.queueFamilyIndexCount = 2U;
            swapchain_create_info.pQueueFamilyIndices = queue_family.data();
        } else {
            swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapchain_create_info.queueFamilyIndexCount = 0U;
        }

        swapchain_create_info.surface = surface;
        swapchain_create_info.minImageCount = selected_swapchain_image_count;
        swapchain_create_info.imageFormat = m_SurfaceFormat.format;
        swapchain_create_info.imageColorSpace = m_SurfaceFormat.colorSpace;
        swapchain_create_info.imageExtent = m_Extent;
        swapchain_create_info.presentMode = presentation_mode;
        swapchain_create_info.imageArrayLayers = 1U;
        swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchain_create_info.preTransform =
            swapchain_support.surface_capabilities.currentTransform;

        swapchain_create_info.compositeAlpha =
            selectCompositeAlpha(swapchain_support.surface_capabilities);

        swapchain_create_info.clipped = VK_TRUE;
        swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

        const VkResult result = vkCreateSwapchainKHR(
            m_Device, &swapchain_create_info, nullptr, &m_Swapchain);

        if (result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanSwapchainCreationFailed,
                "Failed to create a Vulkan swapchain",
                Core::Error::NativeError(static_cast<int>(result),
                                         std::string(toString(result))),
                "Create Vulkan Swapchain");
        }
    }

    VulkanSwapchain::~VulkanSwapchain() noexcept {
        if (m_Swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
        }
    }

    auto VulkanSwapchain::nativeHandle() const noexcept -> VkSwapchainKHR {
        return m_Swapchain;
    }

    auto VulkanSwapchain::surfaceFormat() const noexcept -> VkSurfaceFormatKHR {
        return m_SurfaceFormat;
    }

    auto VulkanSwapchain::extent() const noexcept -> VkExtent2D {
        return m_Extent;
    }
} // namespace SNE::Engine::Renderer::Vulkan
