#pragma once

#include "engine/renderer/pipeline/graphics_pipeline_state.hpp"
#include "engine/renderer/shader/graphics_shader_data.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_graphics_pipeline.hpp"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Identifies a Vulkan vertex buffer binding configuration.
     *
     * Stores the binding index, vertex stride, and input rate
     * without retaining pointers to Vulkan creation structures.
     */
    struct VulkanVertexBindingKey {
        /** @brief Vertex buffer binding index. */
        std::uint32_t binding{};

        /** @brief Number of bytes between consecutive vertex records. */
        std::uint32_t stride{};

        /** @brief Frequency at which vertex attributes advance. */
        VkVertexInputRate input_rate{};

        /**
         * @brief Compares two vertex binding configurations.
         *
         * @param other Vertex binding to compare.
         *
         * @return True when all binding properties match.
         */
        [[nodiscard]] constexpr auto
        operator==(const VulkanVertexBindingKey &other) const noexcept
            -> bool = default;
    };

    /**
     * @brief Identifies a Vulkan vertex attribute configuration.
     *
     * Stores the attribute location, buffer binding, format,
     * and byte offset.
     */
    struct VulkanVertexAttributeKey {
        /** @brief Shader input location. */
        std::uint32_t location{};

        /** @brief Vertex buffer binding containing the attribute. */
        std::uint32_t binding{};

        /** @brief Vulkan format of the attribute. */
        VkFormat format{VK_FORMAT_UNDEFINED};

        /** @brief Attribute byte offset within a vertex record. */
        std::uint32_t offset{};

        /**
         * @brief Compares two vertex attribute configurations.
         *
         * @param other Vertex attribute to compare.
         *
         * @return True when all attribute properties match.
         */
        [[nodiscard]] constexpr auto
        operator==(const VulkanVertexAttributeKey &other) const noexcept
            -> bool = default;
    };

    /**
     * @brief Identifies a Vulkan push-constant range.
     *
     * Stores shader-stage visibility and the covered byte range.
     */
    struct VulkanPushConstantRangeKey {
        /** @brief Shader stages permitted to access the range. */
        VkShaderStageFlags stage_flags{};

        /** @brief Starting byte offset. */
        std::uint32_t offset{};

        /** @brief Number of bytes covered by the range. */
        std::uint32_t size{};

        /**
         * @brief Compares two push-constant ranges.
         *
         * @param other Push-constant range to compare.
         *
         * @return True when stage flags, offset, and size match.
         */
        [[nodiscard]] constexpr auto
        operator==(const VulkanPushConstantRangeKey &other) const noexcept
            -> bool = default;
    };

    /**
     * @brief Identifies a Vulkan graphics pipeline configuration.
     *
     * Stores the configuration needed to distinguish graphics
     * pipeline variants created by the current Vulkan backend.
     *
     * Shader bytecode and configuration arrays are owned by the key.
     * Device and descriptor-set layout handles are borrowed identities
     * and do not transfer ownership of Vulkan resources.
     *
     * Equality compares all members exactly, including settings that
     * may be inactive in a particular pipeline configuration.
     *
     * @note A cache using this key must manage descriptor-set layout
     * lifetime and prevent stale handles from being reused as identities.
     */
    struct VulkanGraphicsPipelineVariantKey {
        /** @brief Logical device on which the pipeline is created. */
        VkDevice device{VK_NULL_HANDLE};

        /** @brief Owned vertex and fragment shader configurations. */
        Shader::GraphicsShaderData shaders{};

        /** @brief Backend-independent graphics pipeline configuration. */
        GraphicsPipelineState state{};

        /** @brief Color and depth attachment formats. */
        GraphicsAttachmentFormats attachment_formats{};

        /** @brief Owned vertex buffer binding descriptions. */
        std::vector<VulkanVertexBindingKey> vertex_bindings;

        /** @brief Owned vertex attribute descriptions. */
        std::vector<VulkanVertexAttributeKey> vertex_attributes;

        /** @brief Owned pipeline push-constant ranges. */
        std::vector<VulkanPushConstantRangeKey> push_constant_ranges;

        /**
         * @brief Descriptor-set layout handles ordered by set index.
         *
         * Handles are non-owning and must not be treated as
         * persistent identifiers after their resources are destroyed.
         */
        std::vector<VkDescriptorSetLayout> descriptor_set_layouts;

        /**
         * @brief Compares two graphics pipeline variant identities.
         *
         * Compares device identity, shaders, graphics state,
         * attachment formats, vertex input, push constants,
         * and descriptor-set layout handles.
         *
         * @param other Pipeline variant key to compare.
         *
         * @return True when all identity members match.
         */
        [[nodiscard]] auto
        operator==(const VulkanGraphicsPipelineVariantKey &other) const
            -> bool = default;
    };

    /**
     * @brief Builds an owning graphics pipeline variant identity.
     *
     * Copies shader data, graphics state, attachment formats,
     * vertex input descriptions, and push-constant ranges.
     *
     * Descriptor-set layout handles are copied without taking
     * ownership of the underlying Vulkan resources.
     *
     * No GPU resources are created.
     *
     * @param device Logical device identifying the pipeline's device.
     * @param attachment_formats Color and depth attachment formats.
     * @param graphics_shader_data Vertex and fragment shader inputs.
     * @param graphics_vertex_input_data Borrowed vertex input descriptions.
     * @param graphics_pipeline_layout_data Borrowed layout requirements.
     * @param state Graphics pipeline state.
     *
     * @return An owning value description of the pipeline variant.
     *
     * @pre Descriptor-set layout references must be non-null and
     * contain valid native handles.
     *
     * @note The returned key does not own any Vulkan handles.
     */
    [[nodiscard]] auto makeVulkanGraphicsPipelineVariantKey(
        VkDevice device, GraphicsAttachmentFormats attachment_formats,
        const Shader::GraphicsShaderData &graphics_shader_data,
        const GraphicsVertexInputData &graphics_vertex_input_data,
        const GraphicsPipelineLayoutData &graphics_pipeline_layout_data,
        GraphicsPipelineState state) -> VulkanGraphicsPipelineVariantKey;
} // namespace SNE::Engine::Renderer::Vulkan
