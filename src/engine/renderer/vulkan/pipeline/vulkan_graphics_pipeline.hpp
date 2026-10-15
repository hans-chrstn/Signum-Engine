#pragma once

#include "engine/renderer/pipeline/graphics_pipeline_state.hpp"
#include "engine/renderer/shader/graphics_shader_data.hpp"
#include <cstdint>
#include <span>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    class VulkanDescriptorSetLayout;

    /**
     * @brief Describes resource interface requirements for a graphics pipeline.
     *
     * Contains borrowed descriptor-set layouts and push-constant ranges
     * used during pipeline-layout creation.
     *
     * The caller must keep these descriptions valid throughout construction.
     */
    struct GraphicsPipelineLayoutData {
        /**
         * @brief Borrowed push-constant ranges required by the pipeline.
         */
        std::span<const VkPushConstantRange> push_constant_ranges;

        /**
         * @brief Borrowed descriptor-set layouts ordered by descriptor set
         * index.
         *
         * Each entry references a descriptor-set layout containing both its
         * native Vulkan handle and retained binding descriptions.
         *
         * The referenced layout objects must remain valid throughout pipeline
         * construction. The graphics pipeline does not take ownership of them.
         *
         * An empty span indicates that no descriptor sets are used.
         *
         * @note Entries must not be null. Descriptor set indices correspond
         * to their positions in the span.
         */
        std::span<const VulkanDescriptorSetLayout *const>
            descriptor_set_layouts;
    };

    /**
     * @brief Describes vertex input bindings and attributes for a graphics
     * pipeline.
     *
     * Contains non-owning views of Vulkan vertex binding and attribute
     * descriptions.
     *
     * The caller retains ownership of the underlying descriptions and must keep
     * them valid throughout graphics-pipeline construction.
     *
     * Empty spans indicate that no vertex bindings or attributes are required.
     *
     * The descriptions are not retained after pipeline creation.
     */
    struct GraphicsVertexInputData {
        /**
         * @brief Borrowed vertex binding descriptions defining buffer slots,
         * strides, and input rates.
         */
        std::span<const VkVertexInputBindingDescription> bindings;

        /**
         * @brief Borrowed vertex attribute descriptions defining shader input
         * locations, formats, bindings, and byte offsets.
         */
        std::span<const VkVertexInputAttributeDescription> attributes;
    };

    /**
     * @brief Describes the attachment formats required by a graphics pipeline.
     *
     * Specifies the color and depth attachment formats used to establish
     * compatibility with Vulkan dynamic rendering.
     */
    struct GraphicsAttachmentFormats {
        /** @brief Format of the color attachment. */
        VkFormat color{VK_FORMAT_UNDEFINED};

        /** @brief Format of the depth attachment. */
        VkFormat depth{VK_FORMAT_UNDEFINED};

        /**
         * @brief Compares two graphics attachment format descriptions.
         *
         * @param other Attachment formats to compare.
         *
         * @return True when color and depth attachment formats match.
         */
        [[nodiscard]] constexpr auto
        operator==(const GraphicsAttachmentFormats &other) const noexcept
            -> bool = default;
    };

    /**
     * @brief Owns a Vulkan graphics pipeline and its pipeline layout.
     *
     * Creates and manages the lifetime of a graphics pipeline used for drawing
     * with Vulkan dynamic rendering.
     *
     * The logical device is borrowed and must remain valid for the lifetime of
     * this object.
     *
     * The graphics pipeline and pipeline layout are owned by this object and
     * are destroyed when the object is destroyed.
     */
    class VulkanGraphicsPipeline {
      private:
        /**
         * @brief Logical device used to create and destroy pipeline resources.
         *
         * The handle is borrowed and must remain valid for the lifetime of this
         * object.
         */
        VkDevice m_Device{VK_NULL_HANDLE};

        /**
         * @brief Vulkan pipeline layout owned by this object.
         */
        VkPipelineLayout m_PipelineLayout{VK_NULL_HANDLE};

        /**
         * @brief Vulkan graphics pipeline handle owned by this object.
         */
        VkPipeline m_Pipeline{VK_NULL_HANDLE};

        /**
         * @brief Releases the currently owned pipeline resources.
         *
         * Destroys the owned graphics pipeline before destroying the owned
         * pipeline layout, then resets all stored Vulkan handles to an empty
         * non-owning state.
         *
         * Safe to call when no pipeline resources are owned.
         */
        auto destroy() noexcept -> void;

      public:
        /**
         * @brief Creates a Vulkan graphics pipeline using supplied SPIR-V
         * shaders, vertex input descriptions, and pipeline-layout requirements.
         *
         * Creates and owns the pipeline layout and graphics pipeline used for
         * Vulkan dynamic rendering with the supplied color and depth
         * attachment formats.
         *
         * The logical device is borrowed and must remain valid for the lifetime
         * of this object.
         *
         * Shader data, vertex input descriptions, and pipeline-layout
         * descriptions are borrowed only during construction and are not
         * retained by the graphics pipeline.
         *
         * Shader entry points, reflected push-constant requirements, and
         * descriptor bindings are validated against the supplied pipeline
         * layout before Vulkan pipeline resources are created.
         *
         * Descriptor-set layouts are borrowed during construction. Their
         * retained binding descriptions are used to validate descriptor
         * set indices, binding indices, descriptor types, counts, and
         * shader-stage visibility.
         *
         * The physical device's push-constant size limit is supplied by the
         * caller. The constructor does not query physical-device properties
         * or retain the supplied limit.
         *
         * @param device Logical device used to create and destroy pipeline
         * resources.
         * @param attachment_formats Color and depth attachment formats
         * required for compatibility with Vulkan dynamic rendering.
         * @param graphics_shader_data Vertex and fragment shader inputs,
         * including owned SPIR-V bytecode and selected entry points.
         * @param graphics_vertex_input_data Borrowed vertex input binding and
         * attribute descriptions.
         * @param graphics_pipeline_layout_data Borrowed pipeline-layout
         * requirements, including push-constant ranges.
         * @param max_push_constants_size Maximum push-constant storage size
         * in bytes supported by the selected physical device.
         * @param state Backend-independent graphics pipeline state used to
         * configure rendering behavior. defaults to depth testing and depth
         * writing enabled.
         *
         * @pre device must be a valid Vulkan logical-device handle.
         * @pre attachment_formats.color must be a defined color
         * attachment format.
         * @pre attachment_formats.depth must be a defined depth
         * attachment format compatible with the active rendering configuration.
         * @pre graphics_shader_data.vertex must specify a vertex-stage
         * shader with valid, nonempty SPIR-V bytecode.
         * @pre graphics_shader_data.fragment must specify a fragment-stage
         * shader with valid, nonempty SPIR-V bytecode.
         * @pre Both shader inputs must specify nonempty exported entry-point
         * names that exist in their respective SPIR-V modules and match
         * their requested execution stages.
         * @pre max_push_constants_size must be a valid, nonzero device limit.
         * @pre Pipeline push-constant ranges must satisfy Vulkan alignment,
         * stage-usage, and physical-device limit requirements.
         * @pre Descriptor-set layout references must be non-null and
         * refer to valid Vulkan descriptor-set layout handles.
         *
         * @post Successful construction produces a non-null Vulkan pipeline
         * layout and graphics pipeline.
         *
         * @throws Core::Error::EngineError if shader validation or reflection
         * fails, shader push-constant or descriptor requirements are
         * incompatible with the pipeline layout, or Vulkan resource
         * creation fails.
         *
         * @note Violated input contracts trigger precondition assertions.
         */
        VulkanGraphicsPipeline(
            VkDevice device, GraphicsAttachmentFormats attachment_formats,
            const Shader::GraphicsShaderData &graphics_shader_data,
            const GraphicsVertexInputData &graphics_vertex_input_data,
            const GraphicsPipelineLayoutData &graphics_pipeline_layout_data,
            std::uint32_t max_push_constants_size,
            GraphicsPipelineState state = {});

        VulkanGraphicsPipeline(const VulkanGraphicsPipeline &) = delete;

        auto operator=(const VulkanGraphicsPipeline &)
            -> VulkanGraphicsPipeline & = delete;

        /**
         * @brief Transfers ownership of graphics-pipeline resources.
         *
         * Transfers the logical-device handle, pipeline layout, and graphics
         * pipeline from the source object.
         *
         * The source object is left in a valid moved-from state containing no
         * owned Vulkan pipeline resources.
         *
         * @param other Pipeline object whose resources are transferred.
         */
        VulkanGraphicsPipeline(VulkanGraphicsPipeline &&other) noexcept;

        /**
         * @brief Replaces the currently owned resources by moving from another
         * pipeline object.
         *
         * Releases any pipeline resources currently owned by this object before
         * transferring the source object's logical-device handle, pipeline
         * layout, and graphics pipeline.
         *
         * Self-move assignment has no effect.
         *
         * The source object is left in a valid moved-from state containing no
         * owned Vulkan pipeline resources.
         *
         * @param other Pipeline object whose resources are transferred.
         *
         * @return Reference to this pipeline object.
         */
        auto operator=(VulkanGraphicsPipeline &&other) noexcept
            -> VulkanGraphicsPipeline &;

        /**
         * @brief Destroys the owned graphics pipeline and pipeline layout.
         */
        ~VulkanGraphicsPipeline() noexcept;

        /**
         * @brief Returns the underlying Vulkan graphics pipeline handle.
         *
         * The returned handle is non-owning and remains valid only while this
         * object owns the graphics pipeline.
         *
         * @return Vulkan graphics pipeline handle owned by this object.
         */
        [[nodiscard]] auto nativeHandle() const noexcept -> VkPipeline;

        /**
         * @brief Returns the Vulkan pipeline layout owned by this graphics
         * pipeline.
         *
         * The returned handle is non-owning and remains valid only while this
         * object owns the pipeline layout.
         *
         * @return Vulkan pipeline layout handle.
         */
        [[nodiscard]] auto layoutHandle() const noexcept -> VkPipelineLayout;
    };

    /**
     * @brief Validates shader inputs required by a Vulkan graphics pipeline.
     *
     * Verifies that the vertex and fragment shader inputs specify their
     * expected execution stages and provide nonempty entry-point names.
     *
     * Validation does not require a Vulkan device or create GPU resources.
     *
     * @param data Shader inputs to validate.
     *
     * @pre data must specify valid vertex and fragment shader stages
     * and nonempty entry-point names.
     *
     * @post Successful validation leaves the shader inputs unchanged.
     *
     * @note Does not verify SPIR-V bytecode contents or whether the
     * selected entry points exist in the compiled modules.
     *
     * @note Invalid configuration triggers a fatal precondition assertion
     * rather than throwing an exception.
     */
    auto validateGraphicsShaderData(const Shader::GraphicsShaderData &data)
        -> void;
} // namespace SNE::Engine::Renderer::Vulkan
