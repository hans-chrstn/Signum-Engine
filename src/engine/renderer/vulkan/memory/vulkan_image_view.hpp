
#pragma once

#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Owns a Vulkan image view.
     *
     * Creates and manages the lifetime of a VkImageView associated with
     * an existing Vulkan image.
     *
     * An image view describes how Vulkan accesses a range of image
     * subresources, including their format, aspect, mip levels, and
     * array layers.
     *
     * The supplied logical device and referenced image are borrowed.
     * Both must remain valid for the lifetime of this object.
     *
     * The image view does not own the underlying Vulkan image or its
     * memory allocation.
     *
     * The type is non-copyable because it exclusively owns the image-view
     * handle. Ownership may be transferred through move construction
     * or move assignment.
     */
    class VulkanImageView {
      private:
        /**
         * @brief Non-owning logical-device handle used to manage the
         * image view.
         *
         * The originating logical device must outlive this object.
         */
        VkDevice m_Device{VK_NULL_HANDLE};

        /**
         * @brief Vulkan image-view handle owned by this object.
         */
        VkImageView m_ImageView{VK_NULL_HANDLE};

        /**
         * @brief Releases the currently owned Vulkan image view.
         *
         * Destroys the image view when one is owned, then resets the
         * stored image-view and logical-device handles.
         *
         * Does not destroy the referenced Vulkan image or logical device.
         *
         * Safe to call when no image view is owned.
         */
        auto destroy() noexcept -> void;

      public:
        /**
         * @brief Creates a Vulkan image view.
         *
         * Creates and owns an image view using the supplied logical device
         * and Vulkan image-view creation information.
         *
         * The logical device and referenced image are borrowed and must
         * remain valid for the lifetime of the created image view.
         *
         * @param device Logical device used to create and destroy the
         * image view.
         * @param create_info Vulkan image-view creation information.
         *
         * @pre device must be a valid Vulkan logical-device handle.
         * @pre create_info.sType must be
         * VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO.
         * @pre create_info.image must be a valid Vulkan image handle.
         * @pre create_info.format must not be VK_FORMAT_UNDEFINED.
         * @pre create_info.subresourceRange.aspectMask must not be zero.
         * @pre create_info must satisfy Vulkan image-view creation
         * requirements.
         *
         * @post Successful creation produces a non-null Vulkan
         * image-view handle.
         *
         * @throws Core::Error::EngineError if Vulkan image-view creation
         * fails.
         *
         * @note Invalid creation parameters checked by the constructor
         * trigger fatal precondition assertions.
         */
        VulkanImageView(VkDevice device,
                        const VkImageViewCreateInfo &create_info);

        /**
         * @brief Destroys the owned Vulkan image view.
         *
         * Releases the image-view handle without destroying the
         * referenced Vulkan image or logical device.
         */
        ~VulkanImageView() noexcept;

        /**
         * @brief Copy construction is disabled because the object
         * exclusively owns its Vulkan image view.
         */
        VulkanImageView(const VulkanImageView &) = delete;

        /**
         * @brief Copy assignment is disabled because the object
         * exclusively owns its Vulkan image view.
         *
         * @return Reference to this object if assignment were supported.
         */
        auto operator=(const VulkanImageView &) -> VulkanImageView & = delete;

        /**
         * @brief Transfers image-view ownership from another object.
         *
         * Transfers the logical-device and image-view handles from the
         * source object.
         *
         * The source object is left in a valid empty state without an
         * owned image view.
         *
         * @param other Image-view owner from which ownership is transferred.
         */
        VulkanImageView(VulkanImageView &&other) noexcept;

        /**
         * @brief Replaces the owned image view through move assignment.
         *
         * Releases the currently owned image view before transferring
         * ownership from the source object.
         *
         * Self-move assignment has no effect.
         *
         * The source object is left in a valid empty state without an
         * owned image view.
         *
         * @param other Image-view owner from which ownership is transferred.
         *
         * @return Reference to this object.
         */
        auto operator=(VulkanImageView &&other) noexcept -> VulkanImageView &;

        /**
         * @brief Returns the native Vulkan image-view handle.
         *
         * Provides non-owning access to the Vulkan image-view handle.
         *
         * The returned handle remains owned by this object and must
         * not be destroyed by the caller.
         *
         * @return The owned VkImageView handle, or VK_NULL_HANDLE
         * if this object has been moved from.
         */
        [[nodiscard]] auto nativeHandle() const noexcept -> VkImageView;
    };
} // namespace SNE::Engine::Renderer::Vulkan
