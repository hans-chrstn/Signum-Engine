#pragma once

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Owns a Vulkan command pool.
     *
     * Creates and manages the lifetime of a VkCommandPool associated with a
     * specific Vulkan queue family.
     *
     * The logical device supplied during construction is borrowed and must
     * remain valid for the lifetime of the command pool.
     *
     * The type is non-copyable because it exclusively owns the command-pool
     * handle. Ownership may be transferred through move construction or move
     * assignment.
     */
    class VulkanCommandPool {
      private:
        VkDevice m_Device{VK_NULL_HANDLE};
        VkCommandPool m_CommandPool{VK_NULL_HANDLE};

        /**
         * @brief Releases the currently owned Vulkan command pool.
         *
         * Destroys the command pool when one is owned and resets the stored
         * command pool and logical-device handles to their empty states.
         *
         * Calling this function when no command pool is owned has no effect.
         */
        auto destroy() noexcept -> void;

      public:
        /**
         * @brief Creates a Vulkan command pool for a queue family.
         *
         * Creates and owns a Vulkan command pool associated with the supplied
         * queue family.
         *
         * The logical device is borrowed and must remain valid for the lifetime
         * of this command pool.
         *
         * @param device Logical device used to create and later destroy the
         * command pool.
         * @param queue_family_index Queue-family index with which allocated
         * command buffers will be associated.
         *
         * @pre device must be a valid Vulkan logical-device handle.
         *
         * @post Successful creation produces a non-null Vulkan command-pool
         * handle.
         *
         * @throws Core::Error::EngineError if Vulkan fails to create the
         * command pool.
         */
        VulkanCommandPool(VkDevice device, std::uint32_t queue_family_index);

        /**
         * @brief Destroys the owned Vulkan command pool.
         */
        ~VulkanCommandPool() noexcept;

        VulkanCommandPool(const VulkanCommandPool &) = delete;

        auto operator=(const VulkanCommandPool &)
            -> VulkanCommandPool & = delete;

        /**
         * @brief Transfers command-pool ownership from another object.
         *
         * The source object is left without an owned Vulkan command pool and
         * remains safe to destroy.
         *
         * @param other Command-pool owner from which ownership is transferred.
         */
        VulkanCommandPool(VulkanCommandPool &&other) noexcept;

        /**
         * @brief Replaces the currently owned command pool by transferring
         * ownership from another object.
         *
         * Any command pool currently owned by this object is released before
         * ownership is transferred. The source object is left without an owned
         * Vulkan command pool and remains safe to destroy.
         *
         * @param other Command-pool owner from which ownership is transferred.
         *
         * @return Reference to this object.
         */
        auto operator=(VulkanCommandPool &&other) noexcept
            -> VulkanCommandPool &;

        /**
         * @brief Returns the underlying Vulkan command-pool handle.
         *
         * The returned handle is non-owning and remains valid only while this
         * object owns the command pool.
         *
         * @return Vulkan command-pool handle owned by this object.
         */
        [[nodiscard]] auto nativeHandle() const noexcept -> VkCommandPool;

        /**
         * @brief Allocates command buffers from this command pool.
         *
         * Allocates the requested number of Vulkan command buffers using this
         * command pool and the specified command-buffer level.
         *
         * The returned command-buffer handles remain associated with this
         * command pool and are valid only while the pool remains valid.
         *
         * @param level Vulkan command-buffer level to allocate.
         * @param count Number of command buffers to allocate.
         *
         * @pre This object must own a valid Vulkan command pool.
         * @pre count must be greater than zero.
         *
         * @post Every returned command-buffer handle is non-null after a
         * successful Vulkan allocation.
         *
         * @return Command-buffer handles allocated from this command pool.
         *
         * @throws Core::Error::EngineError if Vulkan fails to allocate the
         * command buffers.
         */
        [[nodiscard]] auto allocateCommandBuffers(VkCommandBufferLevel level,
                                                  std::uint32_t count)
            -> std::vector<VkCommandBuffer>;

        /**
         * @brief Resets the command pool and its allocated command buffers.
         *
         * Returns command buffers allocated from this pool to their initial
         * state so they may be recorded again.
         *
         * The caller must ensure that no command buffer allocated from this
         * pool is still pending execution on a Vulkan queue.
         *
         * @pre This object must own a valid Vulkan command pool.
         *
         * @throws Core::Error::EngineError if Vulkan fails to reset the command
         * pool.
         */
        auto reset() -> void;
    };
} // namespace SNE::Engine::Renderer::Vulkan
