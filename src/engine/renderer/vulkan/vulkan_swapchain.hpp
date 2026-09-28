#pragma once

#include "engine/platform/window.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "vulkan_queue_families.hpp"
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Owns a Vulkan swapchain used for presentation.
     *
     * Creates and manages the lifetime of a VkSwapchainKHR associated with a
     * Vulkan presentation surface. Swapchain configuration is selected from the
     * capabilities, surface formats, and presentation modes currently reported
     * for the physical-device and surface combination.
     *
     * The logical device and Vulkan surface supplied during construction are
     * borrowed dependencies and must remain valid for the lifetime of this
     * object.
     *
     * Destruction releases the owned VkSwapchainKHR. The logical device,
     * physical device, and surface are not owned by this object.
     *
     * The type is non-copyable because it exclusively owns a Vulkan swapchain
     * handle.
     */
    class VulkanSwapchain {
      private:
        /** Non-owning logical-device handle used to manage the swapchain. */
        VkDevice m_Device{VK_NULL_HANDLE};
        /** Vulkan swapchain handle owned by this object. */
        VkSwapchainKHR m_Swapchain{VK_NULL_HANDLE};
        /** Surface format selected for the swapchain images. */
        VkSurfaceFormatKHR m_SurfaceFormat{};
        /** Extent selected for the swapchain images. */
        VkExtent2D m_Extent{};

      public:
        /**
         * @brief Creates a Vulkan swapchain for a presentation surface.
         *
         * Queries current swapchain support for the physical-device and surface
         * combination, selects the surface format, presentation mode, image
         * extent, image count, composite-alpha mode, and image-sharing
         * behavior, and creates the resulting Vulkan swapchain.
         *
         * The logical device and surface are borrowed and must remain valid for
         * the lifetime of this object.
         *
         * @param physical_device Physical device whose surface capabilities,
         * formats, and presentation modes are used to configure the swapchain.
         * @param logical_device Logical device used to create and later destroy
         * the swapchain.
         * @param surface Vulkan surface to which swapchain images will be
         * presented.
         * @param queue_family_indices Graphics and presentation queue-family
         * indices used to determine swapchain image-sharing behavior.
         * @param framebuffer_size Current framebuffer dimensions in pixels used
         * when selecting the swapchain image extent.
         * @param presentation_preference Backend-independent presentation
         * behavior requested from the renderer.
         * @param fifo_latest_ready_enabled Whether FIFO latest-ready
         * presentation is enabled on the logical device.
         *
         * @throws std::logic_error if required graphics or presentation
         * queue-family indices are missing or no recognized supported
         * composite-alpha mode can be selected.
         * @throws Core::Error::EngineError if swapchain support cannot be
         * queried or Vulkan fails to create the swapchain.
         */
        VulkanSwapchain(VkPhysicalDevice physical_device,
                        VkDevice logical_device, VkSurfaceKHR surface,
                        const QueueFamilyIndices &queue_family_indices,
                        const Platform::FramebufferSize &framebuffer_size,
                        PresentationPreference presentation_preference,
                        bool fifo_latest_ready_enabled);

        /**
         * @brief Destroys the owned Vulkan swapchain.
         *
         * Releases the VkSwapchainKHR using the logical device supplied during
         * construction.
         */
        ~VulkanSwapchain() noexcept;

        VulkanSwapchain(const VulkanSwapchain &) = delete;
        auto operator=(const VulkanSwapchain &) -> VulkanSwapchain & = delete;

        /**
         * @brief Returns the underlying Vulkan swapchain handle.
         *
         * The returned handle is non-owning and remains valid only while this
         * VulkanSwapchain object remains alive.
         *
         * @return Vulkan swapchain handle owned by this object.
         */
        [[nodiscard]] auto nativeHandle() const noexcept -> VkSwapchainKHR;

        /**
         * @brief Returns the surface format selected for the swapchain images.
         *
         * The returned format describes both the pixel format and color space
         * used by the swapchain images.
         *
         * @return Surface format selected during swapchain creation.
         */
        [[nodiscard]] auto surfaceFormat() const noexcept -> VkSurfaceFormatKHR;

        /**
         * @brief Returns the extent selected for the swapchain images.
         *
         * The extent describes the width and height, in pixels, of each
         * swapchain image.
         *
         * @return Extent selected during swapchain creation.
         */
        [[nodiscard]] auto extent() const noexcept -> VkExtent2D;
    };
} // namespace SNE::Engine::Renderer::Vulkan
