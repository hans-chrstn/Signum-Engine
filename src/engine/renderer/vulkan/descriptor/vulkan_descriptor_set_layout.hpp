#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Describes a binding in a Vulkan descriptor-set layout.
     *
     * Stores the resource type, binding index, descriptor count, and
     * shader-stage visibility needed for descriptor compatibility checks.
     *
     * Contains only values and does not own GPU resources or retain
     * pointers from Vulkan descriptor layout creation structures.
     */
    struct VulkanDescriptorBindingDescription {
        /**
         * @brief Binding index within the descriptor set.
         */
        std::uint32_t binding{};

        /**
         * @brief Vulkan descriptor type required by the binding.
         */
        VkDescriptorType descriptor_type{};

        /**
         * @brief Number of descriptors available at this binding.
         *
         * Must be greater than zero for a valid binding.
         */
        std::uint32_t descriptor_count{};

        /**
         * @brief Shader stages permitted to access this binding.
         *
         * Contains a combination of VkShaderStageFlagBits.
         */
        VkShaderStageFlags stage_flags{};
    };

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
         * @brief Owned metadata describing the descriptor-set layout bindings.
         *
         * Stores copies of binding indices, descriptor types, descriptor
         * counts, and shader-stage visibility from the layout creation
         * descriptions.
         *
         * Does not retain Vulkan creation-info pointers or immutable sampler
         * handles. The metadata is used for shader-interface compatibility
         * validation.
         */
        std::vector<VulkanDescriptorBindingDescription>
            m_DescriptorBindingDescriptions;

        /**
         * @brief Releases the owned descriptor-set layout.
         *
         * Destroys the Vulkan layout when present, resets the stored handles,
         * and clears the retained binding descriptions.
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

        /**
         * @brief Returns the descriptor bindings retained by this layout.
         *
         * Provides a read-only view of the binding descriptions copied
         * during descriptor-set layout construction.
         *
         * The returned view does not own the metadata and must not outlive
         * this object or be used after its metadata is moved or replaced.
         *
         * @return Read-only span of descriptor binding descriptions.
         *
         * @note Returns an empty span when the layout contains no bindings.
         */
        [[nodiscard]] auto bindingDescriptions() const noexcept
            -> std::span<const VulkanDescriptorBindingDescription>;
    };
} // namespace SNE::Engine::Renderer::Vulkan
