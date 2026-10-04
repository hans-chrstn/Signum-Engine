#pragma once

#include "engine/renderer/gpu_memory_usage.hpp"
#include <cstddef>
#include <span>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Describes the creation requirements for a Vulkan buffer.
     *
     * Specifies the size and intended Vulkan usage of a buffer together
     * with the engine-level memory usage policy for its backing allocation.
     *
     * This structure describes creation policy only and does not own Vulkan
     * or VMA resources.
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
         * @brief Intended CPU/GPU access pattern for the backing allocation.
         */
        GpuMemoryUsage memory_usage{GpuMemoryUsage::Device};
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
        /**
         * @brief Size of the owned Vulkan buffer in bytes.
         */
        VkDeviceSize m_Size{0};
        /**
         * @brief Engine-level CPU/GPU access intent for the backing allocation.
         */
        GpuMemoryUsage m_MemoryUsage{GpuMemoryUsage::Device};

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
         * usage policy.
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

        /**
         * @brief Writes CPU data into this upload buffer.
         *
         * Copies the supplied bytes into the buffer's backing allocation
         * beginning at the specified byte offset.
         *
         * This operation is valid only for buffers created with
         * GpuMemoryUsage::Upload.
         *
         * @param bytes Bytes to write into the buffer.
         * @param offset Byte offset within the buffer at which writing begins.
         *
         * @pre The supplied byte span is not empty.
         * @pre The supplied byte range fits entirely within the buffer.
         * @pre This buffer was created with GpuMemoryUsage::Upload.
         */
        auto write(std::span<const std::byte> bytes, VkDeviceSize offset = 0U)
            -> void;

        /**
         * @brief Returns the underlying Vulkan buffer handle.
         *
         * The returned handle is non-owning and remains valid only while this
         * VulkanBuffer owns the underlying buffer.
         *
         * @return Vulkan buffer handle owned by this object.
         */
        [[nodiscard]] auto nativeHandle() const noexcept -> VkBuffer;

        /**
         * @brief Returns the size of the Vulkan buffer in bytes.
         *
         * @return Size of the owned buffer in bytes.
         */
        [[nodiscard]] auto size() const noexcept -> VkDeviceSize;
    };
} // namespace SNE::Engine::Renderer::Vulkan
