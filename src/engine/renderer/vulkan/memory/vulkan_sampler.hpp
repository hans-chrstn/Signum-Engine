#pragma once

#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Owns a Vulkan sampler.
     *
     * Creates and manages a VkSampler that defines how image data is
     * sampled, including filtering, addressing, and mipmap selection.
     *
     * The supplied logical device is borrowed and must remain valid
     * for the lifetime of this object.
     *
     * The type is non-copyable because it exclusively owns the sampler
     * handle. Ownership may be transferred through move construction
     * or move assignment.
     */
    class VulkanSampler {
      private:
        /** @brief Borrowed logical-device handle. */
        VkDevice m_Device{VK_NULL_HANDLE};

        /** @brief Vulkan sampler handle owned by this object. */
        VkSampler m_Sampler{VK_NULL_HANDLE};

        /**
         * @brief Releases the owned Vulkan sampler.
         *
         * Destroys the sampler when present and resets the stored
         * sampler and device handles.
         *
         * Safe to call when no sampler is owned.
         */
        auto destroy() noexcept -> void;

      public:
        /**
         * @brief Creates a Vulkan sampler.
         *
         * @param device Logical device used to create and destroy the sampler.
         * @param create_info Vulkan sampler configuration.
         *
         * @pre device must be a valid logical-device handle.
         * @pre create_info must satisfy Vulkan sampler creation requirements.
         *
         * @post Successful construction produces a non-null sampler handle.
         *
         * @throws Core::Error::EngineError if sampler creation fails.
         */
        VulkanSampler(VkDevice device, const VkSamplerCreateInfo &create_info);

        /** @brief Destroys the owned Vulkan sampler. */
        ~VulkanSampler() noexcept;

        VulkanSampler(const VulkanSampler &) = delete;

        auto operator=(const VulkanSampler &) -> VulkanSampler & = delete;

        /**
         * @brief Transfers ownership of a Vulkan sampler.
         *
         * @param other Sampler whose resources are transferred.
         */
        VulkanSampler(VulkanSampler &&other) noexcept;

        /**
         * @brief Replaces the owned sampler through move assignment.
         *
         * Releases the currently owned sampler before transferring
         * ownership from the source object.
         *
         * Self-move assignment has no effect.
         *
         * @param other Sampler whose resources are transferred.
         * @return Reference to this object.
         */
        auto operator=(VulkanSampler &&other) noexcept -> VulkanSampler &;

        /**
         * @brief Returns the underlying Vulkan sampler handle.
         *
         * @return Owned sampler handle, or VK_NULL_HANDLE when moved from.
         */
        [[nodiscard]] auto nativeHandle() const noexcept -> VkSampler;
    };
} // namespace SNE::Engine::Renderer::Vulkan
