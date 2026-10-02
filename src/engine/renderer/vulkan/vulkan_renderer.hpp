#pragma once

#include "engine/platform/window.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "vulkan_device.hpp"
#include "vulkan_device_features.hpp"
#include "vulkan_device_selection.hpp"
#include "vulkan_frame_resources.hpp"
#include "vulkan_instance.hpp"
#include "vulkan_semaphore.hpp"
#include "vulkan_surface.hpp"
#include "vulkan_swapchain.hpp"
#include <string>
#include <vector>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Owns and coordinates the Vulkan renderer runtime.
     *
     * Establishes the lifetime of the Vulkan instance, presentation surface,
     * selected physical-device state, logical device, swapchain, and command
     * infrastructure required by the renderer.
     *
     * Renderer resources are stored in dependency order so they are constructed
     * from lower-level Vulkan dependencies to higher-level resources and
     * destroyed safely in reverse order.
     *
     * The application window is borrowed during renderer initialization and
     * must outlive the renderer.
     *
     * The type is non-copyable because it owns Vulkan resources with exclusive
     * lifetimes.
     */
    class VulkanRenderer {
      private:
        VulkanInstance m_Instance;
        VulkanSurface m_Surface;
        SelectedPhysicalDevice m_PhysicalDevice;
        LogicalDeviceFeatureConfiguration m_LogicalDeviceFeatureConfiguration;
        VulkanDevice m_Device;
        VulkanSwapchain m_Swapchain;
        /**
         * @brief Presentation-wait semaphores associated with swapchain images.
         *
         * Stores one semaphore for each swapchain image. The semaphore selected
         * by an acquired image index is signaled when rendering for that image
         * completes and is subsequently waited on by presentation.
         *
         * The collection follows swapchain-image lifetime and must be recreated
         * when the corresponding swapchain images are replaced.
         */
        std::vector<VulkanSemaphore> m_RenderFinishedSemaphores;
        std::vector<VulkanFrameResources> m_FrameResources;

      public:
        /**
         * @brief Initializes the Vulkan renderer for an application window.
         *
         * Creates the Vulkan instance and presentation surface, selects a
         * suitable physical device, negotiates logical-device features, creates
         * the logical device and presentation swapchain, and establishes the
         * initial command infrastructure.
         *
         * @param application_name Name reported to Vulkan for the application.
         * @param window Platform window used for Vulkan surface creation and
         * framebuffer sizing.
         * @param presentation_preference Presentation behavior requested by the
         * renderer.
         * @param development_diagnostics_enabled Whether development-oriented
         * Vulkan diagnostics should be enabled.
         *
         * @throws Core::Error::EngineError if a required Vulkan resource or
         * capability cannot be initialized.
         */
        VulkanRenderer(const std::string &application_name,
                       const Platform::Window &window,
                       PresentationPreference presentation_preference,
                       bool development_diagnostics_enabled);

        /**
         * @brief Releases the Vulkan renderer runtime.
         *
         * Owned renderer resources are destroyed automatically in reverse
         * dependency order.
         */
        ~VulkanRenderer() = default;

        VulkanRenderer(const VulkanRenderer &) = delete;

        auto operator=(const VulkanRenderer &) -> VulkanRenderer & = delete;
    };
} // namespace SNE::Engine::Renderer::Vulkan
