#pragma once
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Owns a Vulkan fence used for CPU-to-GPU synchronization.
     *
     * Creates and manages the lifetime of a VkFence associated with a logical
     * Vulkan device.
     *
     * A fence may be created in either the signaled or unsignaled state. The
     * logical device supplied during construction is borrowed and must remain
     * valid for the lifetime of the fence.
     *
     * The type is non-copyable because it exclusively owns the fence handle.
     * Ownership may be transferred through move construction or move
     * assignment.
     */
    class VulkanFence {
      private:
        /** Non-owning logical-device handle used to manage the fence. */
        VkDevice m_Device{VK_NULL_HANDLE};
        /** Vulkan fence handle owned by this object. */
        VkFence m_Fence{VK_NULL_HANDLE};
        /**
         * @brief Releases the currently owned Vulkan fence.
         *
         * Destroys the fence when one is owned and resets the stored fence and
         * logical-device handles to their empty states.
         *
         * Calling this function when no fence is owned has no effect.
         */
        auto destroy() noexcept -> void;

      public:
        /**
         * @brief Creates a Vulkan fence with the requested initial state.
         *
         * Creates a fence using the supplied logical device. The fence begins
         * in the signaled state when initially_signaled is true and in the
         * unsignaled state otherwise.
         *
         * The logical device is borrowed and must remain valid for the lifetime
         * of this object.
         *
         * @param device Logical device used to create and later destroy the
         * fence.
         * @param initially_signaled Whether the fence should begin in the
         * signaled state.
         *
         * @pre device must be a valid Vulkan logical-device handle.
         *
         * @throws Core::Error::EngineError if Vulkan fails to create the fence.
         */
        VulkanFence(VkDevice device, bool initially_signaled);

        /**
         * @brief Destroys the owned Vulkan fence.
         */
        ~VulkanFence() noexcept;

        VulkanFence(const VulkanFence &) = delete;

        auto operator=(const VulkanFence &) -> VulkanFence & = delete;

        /**
         * @brief Transfers fence ownership from another object.
         *
         * The source object is left without an owned Vulkan fence and remains
         * safe to destroy.
         *
         * @param other Fence owner from which ownership is transferred.
         */
        VulkanFence(VulkanFence &&other) noexcept;

        /**
         * @brief Replaces the currently owned fence by transferring ownership
         * from another object.
         *
         * Any fence currently owned by this object is released before ownership
         * is transferred. The source object is left without an owned Vulkan
         * fence and remains safe to destroy.
         *
         * @param other Fence owner from which ownership is transferred.
         *
         * @return Reference to this object.
         */
        auto operator=(VulkanFence &&other) noexcept -> VulkanFence &;

        /**
         * @brief Returns the underlying Vulkan fence handle.
         *
         * The returned handle is non-owning and remains valid only while this
         * object owns the fence.
         *
         * @return Vulkan fence handle owned by this object.
         */
        [[nodiscard]] auto nativeHandle() const noexcept -> VkFence;

        /**
         * @brief Waits until this fence becomes signaled.
         *
         * Blocks the calling CPU thread until the Vulkan fence is signaled,
         * indicating that the associated GPU work has completed.
         *
         * @pre This object must own a valid Vulkan fence.
         *
         * @throws Core::Error::EngineError if Vulkan fails while waiting for
         * the fence.
         */
        auto wait() const -> void;

        /**
         * @brief Resets this fence to the unsignaled state.
         *
         * Resets the Vulkan fence so that it may be associated with a
         * subsequent GPU submission.
         *
         * @pre This object must own a valid Vulkan fence.
         *
         * @throws Core::Error::EngineError if Vulkan fails to reset the fence.
         */
        auto reset() -> void;
    };
} // namespace SNE::Engine::Renderer::Vulkan
