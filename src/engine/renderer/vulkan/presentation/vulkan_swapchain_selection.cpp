#include "vulkan_swapchain_selection.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/platform/window.hpp"
#include <algorithm>
#include <cstdint>
#include <span>

namespace SNE::Engine::Renderer::Vulkan {
    auto containsPresentationMode(
        std::span<const VkPresentModeKHR> presentation_modes,
        VkPresentModeKHR presentation_mode) -> bool {
        return std::ranges::contains(presentation_modes, presentation_mode);
    }

    auto
    selectSurfaceFormat(std::span<const VkSurfaceFormatKHR> surface_formats)
        -> VkSurfaceFormatKHR {
        if (surface_formats.empty()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "Surface-format selection requires at least one available "
                "format");
        }

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

    auto
    selectPresentationMode(PresentationPreference presentation_preference,
                           std::span<const VkPresentModeKHR> presentation_modes,
                           bool fifo_latest_ready_enabled) -> VkPresentModeKHR {
        if (!containsPresentationMode(presentation_modes,
                                      VK_PRESENT_MODE_FIFO_KHR)) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "Presentation-mode selection requires FIFO support");
        }

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

        if (capabilities.minImageExtent.width >
                capabilities.maxImageExtent.width ||
            capabilities.minImageExtent.height >
                capabilities.maxImageExtent.height) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "Swap-extent selection requires valid minimum and maximum "
                "extents");
        }

        if (framebuffer_size.width < 0 || framebuffer_size.height < 0) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "Swap-extent selection requires non-negative framebuffer "
                "dimensions");
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

    auto selectSwapchainImageCount(const VkSurfaceCapabilitiesKHR &capabilities)
        -> std::uint32_t {
        if (capabilities.maxImageCount != 0U &&
            capabilities.maxImageCount < capabilities.minImageCount) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "Swapchain image-count selection requires a valid minimum and "
                "maximum image count");
        }

        if (capabilities.minImageCount == UINT32_MAX) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "Swapchain minimum image count cannot be incremented safely");
        }

        const std::uint32_t preferred_count = capabilities.minImageCount + 1U;
        // A maximum of zero means the surface imposes no explicit image-count
        // limit.
        if (capabilities.maxImageCount != 0U &&
            capabilities.maxImageCount < preferred_count) {
            return capabilities.maxImageCount;
        }

        return preferred_count;
    }

    auto selectCompositeAlpha(const VkSurfaceCapabilitiesKHR &capabilities)
        -> VkCompositeAlphaFlagBitsKHR {
        if ((capabilities.supportedCompositeAlpha &
             VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) != 0U) {
            return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        }

        if ((capabilities.supportedCompositeAlpha &
             VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) != 0U) {
            return VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
        }

        if ((capabilities.supportedCompositeAlpha &
             VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR) != 0U) {
            return VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
        }

        if ((capabilities.supportedCompositeAlpha &
             VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) != 0U) {
            return VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
        }

        Core::Assertion::failAssertion(
            Core::Assertion::AssertionType::Precondition,
            Core::Error::Subsystem::Vulkan,
            "Composite-alpha selection requires at least one supported mode");
    }
} // namespace SNE::Engine::Renderer::Vulkan
