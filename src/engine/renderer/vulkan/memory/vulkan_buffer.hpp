#pragma once

#include "engine/renderer/gpu_memory_usage.hpp"
#include "engine/renderer/vulkan/memory/vulkan_memory_allocator.hpp"
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
     * The supplied VulkanMemoryAllocator is borrowed. The allocator and its
     * underlying allocation backend must remain valid for the lifetime of this
     * object.
     *
     * The type is non-copyable because it exclusively owns the Vulkan buffer
     * and its associated allocation.
     */
    class VulkanBuffer {
      private:
        /**
         * @brief Non-owning native allocator handle used internally for buffer
         * memory operations.
         *
         * Obtained from the borrowed VulkanMemoryAllocator during construction.
         * The originating VulkanMemoryAllocator must outlive this VulkanBuffer.
         */
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
         * @brief Vulkan usage flags describing the operations permitted for
         * this buffer.
         */
        VkBufferUsageFlags m_Usage{};
        /**
         * @brief Engine-level CPU/GPU access intent for the backing allocation.
         */
        GpuMemoryUsage m_MemoryUsage{GpuMemoryUsage::Device};

        /**
         * @brief Releases the currently owned Vulkan buffer and backing
         * allocation.
         *
         * Destroys the owned buffer and its VMA allocation when present, then
         * resets the stored allocator handle, Vulkan buffer handle, allocation
         * handle, and associated metadata to their empty defaults.
         *
         * Safe to call when no buffer resources are owned.
         */
        auto destroy() noexcept -> void;

      public:
        /**
         * @brief Creates a Vulkan buffer and its backing allocation.
         *
         * Creates a Vulkan buffer using the supplied memory allocator and
         * applies the engine memory policy derived from the requested GPU
         * memory usage.
         *
         * The allocator is borrowed and must remain valid for the lifetime of
         * this buffer.
         *
         * @param allocator Vulkan memory allocator used to create and destroy
         * the buffer allocation.
         * @param create_info Buffer size, usage flags, and memory-usage policy.
         *
         * @pre create_info.size must be greater than zero.
         * @pre create_info.usage must contain at least one Vulkan buffer usage
         * flag.
         *
         * @post Successful creation produces a non-null Vulkan buffer handle.
         * @post Successful creation produces a valid VMA allocation.
         *
         * @throws Core::Error::EngineError if VMA fails to create the buffer.
         */
        VulkanBuffer(const VulkanMemoryAllocator &allocator,
                     const VulkanBufferCreateInfo &create_info);

        /**
         * @brief Releases the owned Vulkan buffer and its backing allocation.
         */
        ~VulkanBuffer() noexcept;

        VulkanBuffer(const VulkanBuffer &) = delete;

        auto operator=(const VulkanBuffer &) -> VulkanBuffer & = delete;

        /**
         * @brief Transfers ownership of a Vulkan buffer and its backing
         * allocation.
         *
         * Transfers the allocator reference, Vulkan buffer handle, VMA
         * allocation, and associated buffer metadata from the source object.
         *
         * The source object is left in a valid empty state containing no owned
         * Vulkan buffer or allocation.
         *
         * @param other Buffer object whose resources are transferred.
         */
        VulkanBuffer(VulkanBuffer &&other) noexcept;

        /**
         * @brief Replaces the currently owned buffer by moving from another
         * object.
         *
         * Releases any buffer resources currently owned by this object before
         * transferring the source object's allocator reference, Vulkan buffer,
         * VMA allocation, and associated metadata.
         *
         * Self-move assignment has no effect.
         *
         * The source object is left in a valid empty state containing no owned
         * Vulkan buffer or allocation.
         *
         * @param other Buffer object whose resources are transferred.
         *
         * @return Reference to this buffer object.
         */
        auto operator=(VulkanBuffer &&other) noexcept -> VulkanBuffer &;

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
         * @brief Reads data from this readback buffer into CPU memory.
         *
         * Copies bytes from the buffer's backing allocation into the supplied
         * output span beginning at the specified byte offset.
         *
         * This operation is valid only for buffers created with
         * GpuMemoryUsage::Readback.
         *
         * @param output Destination span that receives the bytes read from the
         * buffer.
         * @param offset Byte offset within the buffer at which reading begins.
         *
         * @pre The supplied output span is not empty.
         * @pre The requested byte range fits entirely within the buffer.
         * @pre This buffer was created with GpuMemoryUsage::Readback.
         */
        auto read(std::span<std::byte> output, VkDeviceSize offset = 0U)
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

        /**
         * @brief Returns the Vulkan usage flags assigned to this buffer.
         *
         * The returned flags describe the Vulkan operations for which the
         * buffer was created, such as transfer source, transfer destination,
         * vertex-buffer, or index-buffer usage.
         *
         * @return Vulkan buffer usage flags assigned during construction.
         */
        [[nodiscard]] auto usage() const noexcept -> VkBufferUsageFlags;
    };
} // namespace SNE::Engine::Renderer::Vulkan
