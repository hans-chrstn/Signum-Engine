#pragma once

#include "engine/renderer/presentation_preference.hpp"
#include <cstdint>
#include <span>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Platform {
    struct FramebufferSize;
}

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Determines whether a Vulkan presentation mode is available.
     *
     * Searches the presentation modes reported for the Vulkan surface for the
     * requested presentation mode.
     *
     * @param presentation_modes Contiguous sequence of presentation modes
     * reported as available for the Vulkan surface.
     * @param presentation_mode Presentation mode to search for.
     *
     * @return true if the requested presentation mode is available; otherwise
     * false.
     */
    [[nodiscard]] auto containsPresentationMode(
        std::span<const VkPresentModeKHR> presentation_modes,
        VkPresentModeKHR presentation_mode) -> bool;

    /**
     * @brief Selects Signum's preferred surface format from the available
     * Vulkan surface formats.
     *
     * Applies the renderer's swapchain surface-format selection policy. The
     * preferred SDR format and color-space combination is selected when
     * available; otherwise, the first supplied format is selected as the
     * deterministic fallback.
     *
     * @param surface_formats Contiguous sequence of surface formats reported as
     * available for the Vulkan surface.
     *
     * @pre surface_formats must not be empty.
     *
     * @return Surface format selected for swapchain creation.
     */
    [[nodiscard]] auto
    selectSurfaceFormat(std::span<const VkSurfaceFormatKHR> surface_formats)
        -> VkSurfaceFormatKHR;

    /**
     * @brief Selects a Vulkan presentation mode for the requested presentation
     * preference.
     *
     * Applies the Vulkan backend's presentation-mode selection policy using the
     * presentation modes available for the surface. Preferred modes are
     * selected when available and usable; otherwise, the policy selects an
     * appropriate fallback.
     *
     * FIFO latest-ready presentation is considered usable only when its
     * required device feature is enabled.
     *
     * FIFO presentation serves as the baseline fallback because Vulkan
     * guarantees support for VK_PRESENT_MODE_FIFO_KHR for a valid surface.
     *
     * @param presentation_preference Backend-independent presentation behavior
     * requested from the renderer.
     * @param presentation_modes Contiguous sequence of presentation modes
     * reported as available for the Vulkan surface.
     * @param fifo_latest_ready_enabled Whether FIFO latest-ready presentation
     * is enabled on the Vulkan logical device.
     *
     * @pre presentation_modes must contain VK_PRESENT_MODE_FIFO_KHR.
     *
     * @return Vulkan presentation mode selected for swapchain creation.
     */
    [[nodiscard]] auto
    selectPresentationMode(PresentationPreference presentation_preference,
                           std::span<const VkPresentModeKHR> presentation_modes,
                           bool fifo_latest_ready_enabled) -> VkPresentModeKHR;

    /**
     * @brief Selects the swapchain image extent for the Vulkan surface.
     *
     * Uses the surface's required current extent when one is specified.
     * Otherwise, selects an extent from the current framebuffer dimensions and
     * constrains it to the minimum and maximum extents supported by the
     * surface.
     *
     * @param capabilities Surface capabilities reported for the Vulkan surface.
     * @param framebuffer_size Current framebuffer dimensions in pixels.
     *
     * @pre When the surface does not provide a fixed extent,
     *      minImageExtent must not exceed maxImageExtent.
     * @pre When the surface does not provide a fixed extent,
     *      framebuffer dimensions must be non-negative.
     *
     * @return Extent selected for swapchain images.
     */
    [[nodiscard]] auto
    selectSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities,
                     const Platform::FramebufferSize &framebuffer_size)
        -> VkExtent2D;

    /**
     * @brief Selects the number of images to request for the Vulkan swapchain.
     *
     * Prefers one more image than the minimum required by the surface to
     * provide additional buffering when supported. If the surface defines a
     * maximum image count, the selected count is constrained to that maximum.
     *
     * A maximum image count of zero indicates that the surface does not impose
     * an explicit upper limit.
     *
     * @param capabilities Surface capabilities reported for the Vulkan surface.
     *
     * @pre maxImageCount must be zero or greater than or equal to
     *      minImageCount.
     * @pre minImageCount must allow one additional image to be represented by
     *      std::uint32_t.
     *
     * @return Number of swapchain images to request during swapchain creation.
     */
    [[nodiscard]] auto
    selectSwapchainImageCount(const VkSurfaceCapabilitiesKHR &capabilities)
        -> std::uint32_t;

    /**
     * @brief Selects the composite-alpha mode for the Vulkan swapchain.
     *
     * Prefers opaque composition when supported by the surface and otherwise
     * selects another recognized supported composite-alpha mode.
     *
     * @param capabilities Surface capabilities reported for the Vulkan surface.
     *
     * @return Composite-alpha mode selected for swapchain creation.
     *
     * @pre supportedCompositeAlpha must contain at least one recognized
     * composite-alpha mode.
     */
    [[nodiscard]] auto
    selectCompositeAlpha(const VkSurfaceCapabilitiesKHR &capabilities)
        -> VkCompositeAlphaFlagBitsKHR;
} // namespace SNE::Engine::Renderer::Vulkan
