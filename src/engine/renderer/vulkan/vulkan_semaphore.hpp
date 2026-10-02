#pragma once
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Owns a Vulkan binary semaphore.
     *
     * Creates and manages the lifetime of a VkSemaphore associated with a
     * logical Vulkan device.
     *
     * The logical device supplied during construction is borrowed and must
     * remain valid for the lifetime of the semaphore.
     *
     * The type is non-copyable because it exclusively owns the semaphore
     * handle. Ownership may be transferred through move construction or move
     * assignment.
     */
    class VulkanSemaphore {
      private:
        /** Non-owning logical-device handle used to manage the semaphore. */
        VkDevice m_Device{VK_NULL_HANDLE};
        /** Vulkan semaphore handle owned by this object. */
        VkSemaphore m_Semaphore{VK_NULL_HANDLE};
        /**
         * @brief Releases the currently owned Vulkan semaphore.
         *
         * Destroys the semaphore when one is owned and resets the stored
         * semaphore and logical-device handles to their empty states.
         *
         * Calling this function when no semaphore is owned has no effect.
         */
        auto destroy() noexcept -> void;

      public:
        /**
         * @brief Creates a Vulkan binary semaphore.
         *
         * Creates an unsignaled binary semaphore using the supplied logical
         * device.
         *
         * The logical device is borrowed and must remain valid for the lifetime
         * of this object.
         *
         * @param device Logical device used to create and later destroy the
         * semaphore.
         *
         * @pre device must be a valid Vulkan logical-device handle.
         *
         * @throws Core::Error::EngineError if Vulkan fails to create the
         * semaphore.
         */
        VulkanSemaphore(VkDevice device);

        /**
         * @brief Destroys the owned Vulkan semaphore.
         */
        ~VulkanSemaphore() noexcept;
        VulkanSemaphore(const VulkanSemaphore &) = delete;

        auto operator=(const VulkanSemaphore &) -> VulkanSemaphore & = delete;

        /**
         * @brief Transfers semaphore ownership from another object.
         *
         * The source object is left without an owned Vulkan semaphore and
         * remains safe to destroy.
         *
         * @param other Semaphore owner from which ownership is transferred.
         */
        VulkanSemaphore(VulkanSemaphore &&other) noexcept;

        /**
         * @brief Replaces the currently owned semaphore by transferring
         * ownership from another object.
         *
         * Any semaphore currently owned by this object is released before
         * ownership is transferred. The source object is left without an owned
         * Vulkan semaphore and remains safe to destroy.
         *
         * @param other Semaphore owner from which ownership is transferred.
         *
         * @return Reference to this object.
         */
        auto operator=(VulkanSemaphore &&other) noexcept -> VulkanSemaphore &;

        /**
         * @brief Returns the underlying Vulkan semaphore handle.
         *
         * The returned handle is non-owning and remains valid only while this
         * object owns the semaphore.
         *
         * @return Vulkan semaphore handle owned by this object.
         */
        [[nodiscard]] auto nativeHandle() const noexcept -> VkSemaphore;
    };
} // namespace SNE::Engine::Renderer::Vulkan
