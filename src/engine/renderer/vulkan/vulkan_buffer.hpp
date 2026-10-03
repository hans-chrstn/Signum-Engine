#pragma once
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Describes the creation requirements for a Vulkan buffer.
     *
     * Specifies the size and intended Vulkan usage of a buffer together with
     * the required and preferred memory properties used when selecting its
     * backing allocation.
     *
     * This structure describes creation policy only and does not own Vulkan or
     * VMA resources.
     */
    struct VulkanBufferCreateInfo {
        /**
         * @brief Size of the buffer in bytes.
         */
        VkDeviceSize size{0};

        /**
         * @brief Vulkan usage flags describing how the buffer may be used.
         */
        VkBufferUsageFlags usage{0};

        /**
         * @brief Memory properties that the backing allocation must satisfy.
         */
        VkMemoryPropertyFlags required_memory_properties{0};

        /**
         * @brief Memory properties that should be preferred for the backing
         * allocation when available.
         */
        VkMemoryPropertyFlags preferred_memory_properties{0};
    };

    /**
     * @brief Owns a Vulkan buffer and its VMA-backed memory allocation.
     *
     * Creates and manages the lifetime of a Vulkan buffer together with the
     * allocation that provides its backing memory.
     *
     * The supplied VMA allocator is borrowed and must remain valid for the
     * lifetime of this object.
     *
     * The type is non-copyable because it exclusively owns the Vulkan buffer
     * and its associated allocation.
     */
    class VulkanBuffer {
      private:
        /** Non-owning VMA allocator used to manage the buffer allocation. */
        VmaAllocator m_Allocator{nullptr};
        /** Vulkan buffer handle owned by this object. */
        VkBuffer m_Buffer{VK_NULL_HANDLE};
        /** VMA allocation backing the owned Vulkan buffer. */
        VmaAllocation m_Allocation{nullptr};

      public:
        /**
         * @brief Creates a Vulkan buffer with VMA-managed backing memory.
         *
         * Creates the Vulkan buffer and its associated memory allocation
         * according to the supplied creation description.
         *
         * The allocator is borrowed and must remain valid for the lifetime of
         * this VulkanBuffer.
         *
         * @param allocator VMA allocator used to create and destroy the buffer
         * and its backing allocation.
         * @param create_info Description of the buffer and its memory
         * requirements.
         *
         * @throws Core::Error::EngineError if buffer creation or memory
         * allocation fails.
         */
        VulkanBuffer(VmaAllocator allocator,
                     const VulkanBufferCreateInfo &create_info);

        /**
         * @brief Releases the owned Vulkan buffer and its backing allocation.
         */
        ~VulkanBuffer() noexcept;

        VulkanBuffer(const VulkanBuffer &) = delete;

        auto operator=(const VulkanBuffer &) -> VulkanBuffer & = delete;
    };
} // namespace SNE::Engine::Renderer::Vulkan
