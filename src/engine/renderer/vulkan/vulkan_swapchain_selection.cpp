#include "vulkan_swapchain_selection.hpp"
#include <algorithm>
#include <cstdint>
#include <vector>

namespace SNE::Engine::Renderer::Vulkan {
    auto containsPresentationMode(
        const std::vector<VkPresentModeKHR> &presentation_modes,
        VkPresentModeKHR presentation_mode) -> bool {
        return std::ranges::contains(presentation_modes, presentation_mode);
    }

    auto
    selectSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &surface_formats)
        -> VkSurfaceFormatKHR {
        const VkSurfaceFormatKHR preferred_format{
            .format = VK_FORMAT_B8G8R8A8_SRGB,
            .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        };

        for (const VkSurfaceFormatKHR &surface_format : surface_formats) {
            if (surface_format.format == preferred_format.format &&
                surface_format.colorSpace == preferred_format.colorSpace) {
                return surface_format;
            }
        }
        return surface_formats.front();
    }

    auto selectPresentationMode(
        PresentationPreference presentation_preference,
        const std::vector<VkPresentModeKHR> &presentation_modes,
        bool fifo_latest_ready_enabled) -> VkPresentModeKHR {
        switch (presentation_preference) {
        case PresentationPreference::VSync:
            return VK_PRESENT_MODE_FIFO_KHR;
        case PresentationPreference::LowLatencyVSync:
            if (containsPresentationMode(presentation_modes,
                                         VK_PRESENT_MODE_MAILBOX_KHR)) {
                return VK_PRESENT_MODE_MAILBOX_KHR;
            }
            return VK_PRESENT_MODE_FIFO_KHR;
        case PresentationPreference::AdaptiveVSync:
            if (containsPresentationMode(presentation_modes,
                                         VK_PRESENT_MODE_FIFO_RELAXED_KHR)) {
                return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
            }
            return VK_PRESENT_MODE_FIFO_KHR;
        case PresentationPreference::AllowTearing:
            if (containsPresentationMode(presentation_modes,
                                         VK_PRESENT_MODE_IMMEDIATE_KHR)) {
                return VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
            if (containsPresentationMode(presentation_modes,
                                         VK_PRESENT_MODE_MAILBOX_KHR)) {
                return VK_PRESENT_MODE_MAILBOX_KHR;
            }
            return VK_PRESENT_MODE_FIFO_KHR;
        case PresentationPreference::LatestReadyVSync:
            if (fifo_latest_ready_enabled &&
                containsPresentationMode(
                    presentation_modes,
                    VK_PRESENT_MODE_FIFO_LATEST_READY_KHR)) {
                return VK_PRESENT_MODE_FIFO_LATEST_READY_KHR;
            }
            if (containsPresentationMode(presentation_modes,
                                         VK_PRESENT_MODE_MAILBOX_KHR)) {
                return VK_PRESENT_MODE_MAILBOX_KHR;
            }
            return VK_PRESENT_MODE_FIFO_KHR;
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    auto selectSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities,
                          const Platform::FramebufferSize &framebuffer_size)
        -> VkExtent2D {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        }

        return {
            .width =
                std::clamp(static_cast<std::uint32_t>(framebuffer_size.width),
                           capabilities.minImageExtent.width,
                           capabilities.maxImageExtent.width),
            .height =
                std::clamp(static_cast<std::uint32_t>(framebuffer_size.height),
                           capabilities.minImageExtent.height,
                           capabilities.maxImageExtent.height),
        };
    }
} // namespace SNE::Engine::Renderer::Vulkan
