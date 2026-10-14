#pragma once

#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Owns a Vulkan descriptor-set layout.
     *
     * Creates and manages a descriptor-set layout describing the resource
     * bindings expected by shaders.
     *
     * The logical device is borrowed and must remain valid for the
     * lifetime of this object.
     *
     * The type is non-copyable because it exclusively owns the descriptor-set
     * layout. Ownership may be transferred through move operations.
     */
    class VulkanDescriptorSetLayout {
      private:
        /** @brief Borrowed logical-device handle. */
        VkDevice m_Device{VK_NULL_HANDLE};

        /** @brief Owned Vulkan descriptor-set layout handle. */
        VkDescriptorSetLayout m_DescriptorSetLayout{VK_NULL_HANDLE};

        /**
         * @brief Releases the owned descriptor-set layout.
         *
         * Destroys the layout when present and resets the stored handles.
         *
         * Safe to call when no layout is owned.
         */
        auto destroy() noexcept -> void;

      public:
        /**
         * @brief Creates a Vulkan descriptor-set layout.
         *
         * @param device Logical device used to create and destroy the layout.
         * @param create_info Descriptor bindings and layout configuration.
         *
         * @pre device must be a valid logical-device handle.
         * @pre create_info must satisfy Vulkan descriptor-set layout
         * creation requirements.
         *
         * @post Successful construction produces a non-null layout handle.
         *
         * @throws Core::Error::EngineError if layout creation fails.
         */
        VulkanDescriptorSetLayout(
            VkDevice device,
            const VkDescriptorSetLayoutCreateInfo &create_info);

        /** @brief Destroys the owned descriptor-set layout. */
        ~VulkanDescriptorSetLayout() noexcept;

        VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout &) = delete;

        auto operator=(const VulkanDescriptorSetLayout &)
            -> VulkanDescriptorSetLayout & = delete;

        /**
         * @brief Transfers descriptor-set layout ownership.
         *
         * @param other Layout whose resources are transferred.
         */
        VulkanDescriptorSetLayout(VulkanDescriptorSetLayout &&other) noexcept;

        /**
         * @brief Replaces the owned layout through move assignment.
         *
         * Releases the currently owned layout before transferring ownership.
         * Self-move assignment has no effect.
         *
         * @param other Layout whose resources are transferred.
         * @return Reference to this object.
         */
        auto operator=(VulkanDescriptorSetLayout &&other) noexcept
            -> VulkanDescriptorSetLayout &;

        /**
         * @brief Returns the underlying Vulkan descriptor-set layout handle.
         *
         * @return Owned layout handle, or VK_NULL_HANDLE when moved from.
         */
        [[nodiscard]] auto nativeHandle() const noexcept
            -> VkDescriptorSetLayout;
    };
} // namespace SNE::Engine::Renderer::Vulkan
