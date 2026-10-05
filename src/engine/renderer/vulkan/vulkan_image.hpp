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

      public:
        /**
         * @brief Creates a Vulkan image with allocator-managed backing memory.
         *
         * Creates the Vulkan image and its associated memory allocation
         * according to the supplied creation description.
         *
         * The VulkanMemoryAllocator is borrowed and must remain valid for the
         * lifetime of this VulkanImage.
         *
         * @param allocator Engine-owned Vulkan memory allocator used to create
         * and destroy the image and its backing allocation.
         * @param image_info Description of the image.
         *
         * @throws Core::Error::EngineError if image creation or memory
         * allocation fails.
         */
        VulkanImage(const VulkanMemoryAllocator &allocator,
                    const VulkanImageCreateInfo &image_info);

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
    };
} // namespace SNE::Engine::Renderer::Vulkan
