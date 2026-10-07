#pragma once

#include "vulkan_memory_allocator.hpp"
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Describes the creation requirements for a Vulkan image.
     *
     * Specifies the image type, extent, format, intended Vulkan usage for an
     * image and its backing allocation.
     *
     * This structure describes creation policy only and does not own Vulkan
     * resources or backing allocations.
     */
    struct VulkanImageCreateInfo {
        /**
         * @brief Extent of the image in texels.
         */
        VkExtent3D extent{};

        /**
         * @brief Vulkan image type describing the dimensionality of the image.
         */
        VkImageType image_type{VK_IMAGE_TYPE_2D};

        /**
         * @brief Vulkan format used to store image texels.
         */
        VkFormat format{VK_FORMAT_UNDEFINED};

        /**
         * @brief Vulkan usage flags describing how the image may be used.
         */
        VkImageUsageFlags usage{0U};
    };

    /**
     * @brief Owns a Vulkan image and its allocator-managed memory allocation.
     *
     * Creates and manages the lifetime of a Vulkan image together with the
     * allocation that provides its backing memory.
     *
     * The supplied VulkanMemoryAllocator is borrowed. The allocator and its
     * underlying allocation backend must remain valid for the lifetime of this
     * object.
     *
     * The type is non-copyable because it exclusively owns the Vulkan image
     * and its associated allocation.
     */
    class VulkanImage {
      private:
        /**
         * @brief Non-owning native allocator handle used internally for image
         * memory operations.
         *
         * Obtained from the borrowed VulkanMemoryAllocator during construction.
         * The originating VulkanMemoryAllocator must outlive this VulkanImage.
         */
        VmaAllocator m_Allocator{nullptr};

        /**
         * @brief Vulkan image handle owned by this object.
         */
        VkImage m_Image{VK_NULL_HANDLE};

        /**
         * @brief VMA allocation backing the owned Vulkan image.
         */
        VmaAllocation m_Allocation{nullptr};

        /**
         * @brief Releases the currently owned Vulkan image and backing
         * allocation.
         *
         * Destroys the owned image and its VMA allocation when present, then
         * resets the stored allocator handle, Vulkan image handle, and
         * allocation handle to their empty defaults.
         *
         * Safe to call when no image resources are owned.
         */
        auto destroy() noexcept -> void;

      public:
        /**
         * @brief Creates a Vulkan image and its backing allocation.
         *
         * Creates a Vulkan image using the supplied memory allocator and
         * allocates device-oriented backing memory according to the engine's
         * Vulkan memory policy.
         *
         * The allocator is borrowed and must remain valid for the lifetime of
         * this image.
         *
         * @param allocator Vulkan memory allocator used to create and destroy
         * the image allocation.
         * @param create_info Image type, extent, format, and Vulkan usage
         * flags.
         *
         * @pre create_info.image_type must be a supported Vulkan image type.
         * @pre create_info.extent must satisfy the dimensional requirements of
         * the requested image type.
         * @pre create_info.format must not be VK_FORMAT_UNDEFINED.
         * @pre create_info.usage must contain at least one Vulkan image usage
         * flag.
         *
         * @post Successful creation produces a non-null Vulkan image handle.
         * @post Successful creation produces a valid VMA allocation.
         *
         * @throws Core::Error::EngineError if VMA fails to create the image.
         */
        VulkanImage(const VulkanMemoryAllocator &allocator,
                    const VulkanImageCreateInfo &create_info);

        /**
         * @brief Releases the owned Vulkan image and its backing allocation.
         */
        ~VulkanImage() noexcept;

        /**
         * @brief Copy construction is disabled because the object exclusively
         * owns its Vulkan image and allocation.
         */
        VulkanImage(const VulkanImage &) = delete;

        /**
         * @brief Copy assignment is disabled because the object exclusively
         * owns its Vulkan image and allocation.
         *
         * @return This object if assignment were supported.
         */
        auto operator=(const VulkanImage &) -> VulkanImage & = delete;

        /**
         * @brief Transfers ownership of a Vulkan image and its backing
         * allocation.
         *
         * Transfers the allocator reference, Vulkan image handle, and VMA
         * allocation from the source object.
         *
         * The source object is left in a valid empty state containing no owned
         * Vulkan image or allocation.
         *
         * @param other Image object whose resources are transferred.
         */
        VulkanImage(VulkanImage &&other) noexcept;

        /**
         * @brief Replaces the currently owned image by moving from another
         * object.
         *
         * Releases any image resources currently owned by this object before
         * transferring the source object's allocator reference, Vulkan image
         * handle, and VMA allocation.
         *
         * Self-move assignment has no effect.
         *
         * The source object is left in a valid empty state containing no owned
         * Vulkan image or allocation.
         *
         * @param other Image object whose resources are transferred.
         *
         * @return Reference to this image object.
         */
        auto operator=(VulkanImage &&other) noexcept -> VulkanImage &;
    };
} // namespace SNE::Engine::Renderer::Vulkan
