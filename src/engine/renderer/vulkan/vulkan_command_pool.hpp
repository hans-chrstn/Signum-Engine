#pragma once

#include <cstdint>
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
         * @param device Logical device used to create and later destroy the
         * command pool.
         * @param queue_family_index Queue-family index with which allocated
         * command buffers will be associated.
         *
         * @pre device must be a valid Vulkan logical-device handle.
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
    };
} // namespace SNE::Engine::Renderer::Vulkan
