#include "vulkan_swapchain_selection.hpp"
#include <vector>

namespace SNE::Engine::Renderer::Vulkan {
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
} // namespace SNE::Engine::Renderer::Vulkan
