#pragma once

#include "engine/renderer/presentation_preference.hpp"
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

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
} // namespace SNE::Engine::Renderer::Vulkan
