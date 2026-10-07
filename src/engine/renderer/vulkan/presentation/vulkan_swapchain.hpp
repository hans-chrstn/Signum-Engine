#pragma once

#include "engine/renderer/presentation_preference.hpp"
#include <span>
#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Platform {
    struct FramebufferSize;
}

namespace SNE::Engine::Renderer::Vulkan {
    struct SelectedQueueFamilies;

    /**
     * @brief Owns a Vulkan swapchain and its presentation image views.
     *
     * Creates and manages the lifetime of a VkSwapchainKHR associated with a
     * Vulkan presentation surface. Swapchain configuration is selected from the
     * capabilities, surface formats, and presentation modes currently reported
     * for the physical-device and surface combination.
     *
     * The swapchain-provided VkImage handles are non-owning. Image views
     * created for those images are owned by this object.
     *
     * The logical device and Vulkan surface supplied during construction are
     * borrowed dependencies and must remain valid for the lifetime of this
     * object.
     *
     * Destruction releases the owned VkImageView handles before releasing the
     * owned VkSwapchainKHR. The logical device, physical device, surface, and
     * swapchain-provided images are not owned by this object.
     *
     * The type is non-copyable because it exclusively owns Vulkan resource
     * handles. Ownership may be transferred through move construction or move
     * assignment.
     */
    class VulkanSwapchain {
      private:
        /** Non-owning logical-device handle used to manage the swapchain. */
        VkDevice m_Device{VK_NULL_HANDLE};
        /** Vulkan swapchain handle owned by this object. */
        VkSwapchainKHR m_Swapchain{VK_NULL_HANDLE};
        /** Non-owning handles to the presentable images provided by the
         * swapchain. */
        std::vector<VkImage> m_Images;
        /** Vulkan image views owned for the swapchain's presentable images. */
        std::vector<VkImageView> m_ImageViews;
        /** Surface format selected for the swapchain images. */
        VkSurfaceFormatKHR m_SurfaceFormat{};
        /** Extent selected for the swapchain images. */
        VkExtent2D m_Extent{};
        /**
         * @brief Releases the currently owned swapchain resources.
         *
         * Destroys all owned swapchain image views before destroying the owned
         * swapchain handle, then resets the stored swapchain state to an empty
         * non-owning state.
         *
         * Safe to call when no swapchain resources are currently owned.
         */
        auto destroy() noexcept -> void;

      public:
        /**
         * @brief Creates a Vulkan swapchain and its presentation image views.
         *
         * Queries current swapchain support for the physical-device and surface
         * combination, selects the surface format, presentation mode, image
         * extent, image count, composite-alpha mode, and image-sharing
         * behavior, and creates the resulting Vulkan swapchain.
         *
         * Enumerates the images provided by the created swapchain and creates
         * one two-dimensional color image view for each swapchain image.
         *
         * The logical device and surface are borrowed and must remain valid for
         * the lifetime of this object.
         *
         * @param physical_device Physical device whose surface capabilities,
         * formats, and presentation modes are used to configure the swapchain.
         * @param device Logical device used to create and later destroy the
         * swapchain.
         * @param surface Vulkan surface to which swapchain images will be
         * presented.
         * @param queue_families Selected graphics and presentation queue
         * families whose indices are used to determine swapchain image-sharing
         * behavior.
         * @param framebuffer_size Current framebuffer dimensions in pixels used
         * when selecting the swapchain image extent.
         * @param presentation_preference Backend-independent presentation
         * behavior requested from the renderer.
         * @param fifo_latest_ready_enabled Whether FIFO latest-ready
         * presentation is enabled on the logical device.
         * @param old_swapchain Existing swapchain being replaced during
         * recreation. May be VK_NULL_HANDLE when creating the initial
         * swapchain. The handle is borrowed and is not destroyed directly by
         * this constructor.
         *
         * @pre physical_device must be a valid Vulkan physical-device handle.
         * @pre device must be a valid Vulkan logical-device handle.
         * @pre surface must be a valid Vulkan surface handle.
         * @pre old_swapchain must be VK_NULL_HANDLE or a valid swapchain being
         * replaced for the supplied surface.
         *
         * @post Successful construction produces a non-null Vulkan swapchain
         * handle.
         * @post Successful swapchain image enumeration produces at least one
         * image.
         * @post Every enumerated swapchain image handle is non-null.
         * @post Every created swapchain image-view handle is non-null.
         *
         * @throws Core::Error::EngineError if swapchain support cannot be
         * queried, swapchain creation fails, swapchain images cannot be
         * enumerated, or an image view cannot be created.
         */
        VulkanSwapchain(VkPhysicalDevice physical_device, VkDevice device,
                        VkSurfaceKHR surface,
                        const SelectedQueueFamilies &queue_families,
                        const Platform::FramebufferSize &framebuffer_size,
                        PresentationPreference presentation_preference,
                        bool fifo_latest_ready_enabled,
                        VkSwapchainKHR old_swapchain = VK_NULL_HANDLE);

        /**
         * @brief Destroys the owned Vulkan image views and swapchain.
         *
         * Releases all owned VkImageView handles before releasing the
         * VkSwapchainKHR using the logical device supplied during construction.
         *
         * Swapchain-provided VkImage handles are not destroyed directly because
         * their lifetime is owned by the swapchain.
         */
        ~VulkanSwapchain() noexcept;

        VulkanSwapchain(const VulkanSwapchain &) = delete;
        auto operator=(const VulkanSwapchain &) -> VulkanSwapchain & = delete;

        /**
         * @brief Transfers swapchain ownership from another object.
         *
         * Transfers the borrowed logical-device handle, owned swapchain handle,
         * swapchain-image state, owned image views, selected surface format,
         * and extent from the source object.
         *
         * The source object is left in an empty state that is safe to destroy.
         *
         * @param other Swapchain owner from which ownership is transferred.
         */
        VulkanSwapchain(VulkanSwapchain &&other) noexcept;

        /**
         * @brief Replaces the currently owned swapchain by transferring
         * ownership from another object.
         *
         * Releases any swapchain resources currently owned by this object
         * before taking ownership of the source object's swapchain state.
         *
         * Self-move assignment has no effect. The source object is left in an
         * empty state that is safe to destroy.
         *
         * @param other Swapchain owner from which ownership is transferred.
         *
         * @return Reference to this object.
         */
        auto operator=(VulkanSwapchain &&other) noexcept -> VulkanSwapchain &;

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

        /**
         * @brief Returns a read-only view of the swapchain images.
         *
         * The returned span is non-owning and remains valid only while this
         * VulkanSwapchain owns the underlying image collection.
         *
         * The VkImage handles are provided by the Vulkan swapchain and are not
         * destroyed directly by this object.
         *
         * @return Read-only view of the swapchain image handles.
         */
        [[nodiscard]] auto images() const noexcept -> std::span<const VkImage>;

        /**
         * @brief Returns a read-only view of the swapchain image views.
         *
         * The returned span is non-owning and remains valid only while this
         * VulkanSwapchain owns the underlying image views.
         *
         * @return Read-only view of the swapchain image-view handles.
         */
        [[nodiscard]] auto imageViews() const noexcept
            -> std::span<const VkImageView>;
    };
} // namespace SNE::Engine::Renderer::Vulkan
