#pragma once

#include "engine/renderer/vulkan/descriptor/vulkan_descriptor_set_layout.hpp"
#include <span>
#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Calculates descriptor-pool capacities from layout requirements.
     *
     * Combines descriptor counts for bindings of the same Vulkan
     * descriptor type across all required descriptor sets.
     *
     * @param requirements Descriptor requirements grouped by set index.
     *
     * @return Descriptor-pool sizes grouped by descriptor type.
     *
     * @pre Summed descriptor counts must fit within std::uint32_t.
     */
    [[nodiscard]] auto calculateVkDescriptorPoolSizes(
        std::span<const VulkanDescriptorSetLayoutRequirements> requirements)
        -> std::vector<VkDescriptorPoolSize>;

    /**
     * @brief Owns a Vulkan descriptor pool.
     *
     * Creates and manages a descriptor pool used to allocate descriptor
     * sets for shader resource bindings.
     *
     * The logical device is borrowed and must remain valid for the
     * lifetime of this object.
     *
     * Destroying the pool implicitly frees all descriptor sets
     * allocated from it. Those sets must no longer be in use by
     * the GPU when the pool is destroyed.
     *
     * The type is non-copyable because it exclusively owns the
     * descriptor pool. Ownership may be transferred through move
     * operations.
     */
    class VulkanDescriptorPool {
      private:
        /** @brief Borrowed logical-device handle. */
        VkDevice m_Device{VK_NULL_HANDLE};

        /** @brief Owned Vulkan descriptor-pool handle. */
        VkDescriptorPool m_DescriptorPool{VK_NULL_HANDLE};

        /**
         * @brief Releases the owned descriptor pool.
         *
         * Destroys the pool when present, implicitly freeing
         * its allocated descriptor sets, and resets the stored handles.
         *
         * Safe to call when no pool is owned.
         */
        auto destroy() noexcept -> void;

      public:
        /**
         * @brief Creates a Vulkan descriptor pool.
         *
         * @param device Logical device used to create and destroy the pool.
         * @param create_info Descriptor capacities, maximum set count,
         * and pool creation flags.
         *
         * @pre device must be a valid logical-device handle.
         * @pre create_info must satisfy Vulkan descriptor-pool
         * creation requirements.
         *
         * @post Successful construction produces a non-null pool handle.
         *
         * @throws Core::Error::EngineError if pool creation fails.
         */
        VulkanDescriptorPool(VkDevice device,
                             const VkDescriptorPoolCreateInfo &create_info);

        /** @brief Destroys the owned descriptor pool. */
        ~VulkanDescriptorPool() noexcept;

        VulkanDescriptorPool(const VulkanDescriptorPool &) = delete;

        auto operator=(const VulkanDescriptorPool &)
            -> VulkanDescriptorPool & = delete;

        /**
         * @brief Transfers descriptor-pool ownership.
         *
         * @param other Pool whose resources are transferred.
         */
        VulkanDescriptorPool(VulkanDescriptorPool &&other) noexcept;

        /**
         * @brief Replaces the owned pool through move assignment.
         *
         * Releases the currently owned pool before transferring
         * ownership from the source object.
         *
         * Self-move assignment has no effect.
         *
         * @param other Pool whose resources are transferred.
         * @return Reference to this object.
         */
        auto operator=(VulkanDescriptorPool &&other) noexcept
            -> VulkanDescriptorPool &;

        /**
         * @brief Returns the underlying Vulkan descriptor-pool handle.
         *
         * @return Owned pool handle, or VK_NULL_HANDLE when moved from.
         */
        [[nodiscard]] auto nativeHandle() const noexcept -> VkDescriptorPool;

        /**
         * @brief Allocates descriptor sets from this pool.
         *
         * The returned descriptor-set handles are owned by the pool.
         * They remain valid until the pool is reset or destroyed.
         *
         * The caller must ensure that descriptor sets are no longer
         * in use by the GPU before their pool is destroyed or reset.
         *
         * @param layouts Descriptor-set layouts for the requested allocations.
         * @return Allocated descriptor-set handles.
         *
         * @pre layouts must not be empty.
         * @pre Every layout handle must be valid.
         * @pre The descriptor pool must be valid.
         *
         * @throws Core::Error::EngineError if allocation fails.
         */
        [[nodiscard]] auto
        allocateDescriptorSets(std::span<const VkDescriptorSetLayout> layouts)
            -> std::vector<VkDescriptorSet>;
    };
} // namespace SNE::Engine::Renderer::Vulkan
