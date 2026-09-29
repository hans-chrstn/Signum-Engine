#pragma once

#include "engine/renderer/presentation_preference.hpp"
#include <cstdint>
#include <vector>
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
     * @param presentation_modes Presentation modes reported as available for
     * the Vulkan surface.
     * @param presentation_mode Presentation mode to search for.
     *
     * @return true if the requested presentation mode is available; otherwise
     * false.
     */
    [[nodiscard]] auto containsPresentationMode(
        const std::vector<VkPresentModeKHR> &presentation_modes,
        VkPresentModeKHR presentation_mode) -> bool;

    /**
     * @brief Selects Signum's preferred surface format from the available
     * Vulkan surface formats.
     *
     * Applies the renderer's swapchain surface-format selection policy. The
     * preferred SDR format and color-space combination is selected when
     * available; otherwise, the function selects a deterministic fallback.
     *
     * The supplied collection must contain at least one surface format.
     *
     * @param surface_formats Surface formats reported as available for the
     * Vulkan surface.
     *
     * @return Surface format selected for swapchain creation.
     */
    [[nodiscard]] auto
    selectSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &surface_formats)
        -> VkSurfaceFormatKHR;

    /**
     * @brief Selects a Vulkan presentation mode for the requested presentation
     * preference.
     *
     * Applies the Vulkan backend's presentation-mode selection policy using the
     * presentation modes available for the surface. Preferred modes are
     * selected when they are available and usable; otherwise, the policy
     * selects an appropriate fallback.
     *
     * FIFO latest-ready presentation is considered usable only when its
     * required device feature is enabled.
     *
     * The supplied collection must contain the presentation modes reported for
     * a valid Vulkan surface. FIFO presentation is guaranteed by Vulkan and
     * serves as the baseline fallback mode.
     *
     * @param presentation_preference Backend-independent presentation behavior
     * requested from the renderer.
     * @param presentation_modes Presentation modes reported as available for
     * the Vulkan surface.
     * @param fifo_latest_ready_enabled Whether FIFO latest-ready presentation
     * is enabled on the Vulkan logical device.
     *
     * @return Vulkan presentation mode selected for swapchain creation.
     */
    [[nodiscard]] auto selectPresentationMode(
        PresentationPreference presentation_preference,
        const std::vector<VkPresentModeKHR> &presentation_modes,
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
     * @note An assertion failure is reported if the supplied capabilities
     * contain no recognized supported composite-alpha mode.
     */
    [[nodiscard]] auto
    selectCompositeAlpha(const VkSurfaceCapabilitiesKHR &capabilities)
        -> VkCompositeAlphaFlagBitsKHR;
} // namespace SNE::Engine::Renderer::Vulkan
