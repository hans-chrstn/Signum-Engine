#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Stores swapchain capabilities reported for a physical device and
     * surface.
     *
     * Contains the surface capabilities, available surface formats, and
     * available presentation modes used later when selecting a swapchain
     * configuration.
     */
    struct SwapchainSupportDetails {
        /** Surface capabilities reported for the physical device and surface.
         */
        VkSurfaceCapabilitiesKHR surface_capabilities{};
        /** Surface formats available for swapchain images. */
        std::vector<VkSurfaceFormatKHR> available_surface_formats;
        /** Presentation modes available for the surface. */
        std::vector<VkPresentModeKHR> available_presentation_modes;
    };

    /**
     * @brief Queries swapchain support for a physical device and surface.
     *
     * Retrieves the surface capabilities, available surface formats, and
     * presentation modes exposed for the specified physical-device and surface
     * combination.
     *
     * @param device Physical device whose presentation support is queried.
     * @param surface Vulkan surface for which swapchain support is queried.
     *
     * @return Swapchain support details reported for the physical device and
     * surface.
     *
     * @throws Core::Error::EngineError if a Vulkan swapchain-support query
     * fails.
     */
    [[nodiscard]] auto querySwapchainSupport(VkPhysicalDevice device,
                                             VkSurfaceKHR surface)
        -> SwapchainSupportDetails;
} // namespace SNE::Engine::Renderer::Vulkan
