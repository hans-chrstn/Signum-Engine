#pragma once

#include <cstdint>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Describes basic allocator-wide Vulkan memory allocation
     * statistics.
     *
     * Represents a snapshot of the number of active allocations, the total
     * number of bytes associated with those allocations, and the number of
     * memory blocks currently owned by the allocation backend.
     *
     * The reported values are diagnostic information and may change as
     * resources are created and destroyed.
     */
    struct VulkanMemoryAllocationStatistics {
        /** Number of active Vulkan memory allocations. */
        std::uint32_t allocation_count{};

        /** Total number of bytes associated with active allocations. */
        VkDeviceSize allocation_bytes{};

        /** Number of Vulkan memory blocks currently owned by the allocator. */
        std::uint32_t block_count{};
    };

    /**
     * @brief Describes current memory usage and budget for a Vulkan memory
     * heap.
     *
     * Represents allocator-visible memory consumption together with the current
     * budget available for the corresponding physical-device memory heap.
     *
     * Budget information may use VK_EXT_memory_budget when the extension was
     * enabled for the allocator. Otherwise the allocation backend may provide a
     * less precise estimate.
     */
    struct VulkanMemoryHeapBudget {
        /** Current memory usage associated with the heap, in bytes. */
        VkDeviceSize usage{};
        /** Current memory budget available for the heap, in bytes. */
        VkDeviceSize budget{};
    };

    /**
     * @brief Owns the Vulkan Memory Allocator used by the Vulkan renderer.
     *
     * Creates and manages the lifetime of a VMA allocator associated with a
     * Vulkan instance, physical device, and logical device.
     *
     * The supplied Vulkan handles are borrowed and must remain valid for the
     * lifetime of this object.
     *
     * GPU resources allocated through this allocator must be released before
     * the allocator is destroyed.
     *
     * The type is non-copyable because it exclusively owns the VMA allocator.
     */
    class VulkanMemoryAllocator {
      private:
        VmaAllocator m_Allocator{nullptr};

      public:
        /**
         * @brief Creates the Vulkan memory allocator.
         *
         * Creates and owns a VMA allocator associated with the supplied Vulkan
         * instance, physical device, and logical device.
         *
         * The Vulkan handles are borrowed and must remain valid for the
         * lifetime of this allocator.
         *
         * @param instance Vulkan instance used by the allocator.
         * @param physical_device Physical device whose memory capabilities are
         * used.
         * @param device Logical device used for Vulkan memory operations.
         * @param api_version Vulkan API version used by the renderer.
         * @param memory_budget_extension_enabled Whether VK_EXT_memory_budget
         * was enabled for the supplied logical device and may be used by the
         * allocator.
         *
         * @pre instance must be a valid Vulkan instance handle.
         * @pre physical_device must be a valid Vulkan physical-device handle.
         * @pre device must be a valid Vulkan logical-device handle.
         *
         * @post Successful creation produces a valid VMA allocator.
         *
         * @throws Core::Error::EngineError if the VMA allocator cannot be
         * created.
         */
        VulkanMemoryAllocator(VkInstance instance,
                              VkPhysicalDevice physical_device, VkDevice device,
                              std::uint32_t api_version,
                              bool memory_budget_extension_enabled);

        /**
         * @brief Releases the owned VMA allocator.
         */
        ~VulkanMemoryAllocator() noexcept;
        VulkanMemoryAllocator(const VulkanMemoryAllocator &) = delete;

        auto operator=(const VulkanMemoryAllocator &)
            -> VulkanMemoryAllocator & = delete;

        /**
         * @brief Returns the underlying VMA allocator handle.
         *
         * The returned handle is non-owning and remains valid only while this
         * VulkanMemoryAllocator object remains alive.
         *
         * @return VMA allocator owned by this object.
         */
        [[nodiscard]] auto nativeHandle() const noexcept -> VmaAllocator;

        /**
         * @brief Queries the current memory usage and budget for Vulkan memory
         * heaps.
         *
         * Retrieves allocator-visible memory usage and budget information for
         * each memory heap reported by the physical device.
         *
         * The returned information is a snapshot and may change as allocations
         * are created, destroyed, or external memory pressure changes.
         *
         * @return Current usage and budget information for each Vulkan memory
         * heap.
         */
        [[nodiscard]] auto queryMemoryHeapBudgets() const
            -> std::vector<VulkanMemoryHeapBudget>;

        /**
         * @brief Queries basic allocator-wide Vulkan memory allocation
         * statistics.
         *
         * Calculates a snapshot of the allocator's current memory statistics
         * and returns the allocator-wide totals using an engine-owned
         * representation.
         *
         * The returned values are intended for diagnostics and may change as
         * Vulkan resources are created and destroyed.
         *
         * @return Current allocator-wide Vulkan memory allocation statistics.
         */
        [[nodiscard]] auto queryMemoryAllocationStatistics() const
            -> VulkanMemoryAllocationStatistics;
    };
} // namespace SNE::Engine::Renderer::Vulkan
