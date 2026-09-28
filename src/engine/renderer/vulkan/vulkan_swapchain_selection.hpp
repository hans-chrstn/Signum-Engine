#pragma once
#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
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
} // namespace SNE::Engine::Renderer::Vulkan
