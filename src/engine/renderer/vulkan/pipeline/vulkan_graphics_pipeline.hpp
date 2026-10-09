#pragma once
#include <cstdint>
#include <span>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Describes resource interface requirements for a graphics pipeline.
     *
     * Contains non-owning push-constant range descriptions used during
     * pipeline-layout creation.
     *
     * The caller must keep the descriptions valid throughout construction.
     */
    struct GraphicsPipelineLayoutData {
        /**
         * @brief Borrowed push-constant ranges required by the pipeline.
         */
        std::span<const VkPushConstantRange> push_constant_ranges;
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
     * @brief Describes compiled shader data used to create a graphics pipeline.
     *
     * Contains non-owning views of the SPIR-V bytecode for the vertex and
     * fragment shader stages.
     *
     * The caller retains ownership of the underlying bytecode and must keep
     * it valid throughout graphics-pipeline construction.
     *
     * The shader data is not retained after pipeline creation.
     */
    struct GraphicsShaderData {
        /**
         * @brief Borrowed SPIR-V bytecode for the vertex shader stage.
         */
        std::span<const std::uint32_t> vertex_data;

        /**
         * @brief Borrowed SPIR-V bytecode for the fragment shader stage.
         */
        std::span<const std::uint32_t> fragment_data;
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
         * shaders and vertex input descriptions.
         *
         * Creates and owns the pipeline layout and graphics pipeline used for
         * Vulkan dynamic rendering with the supplied color-attachment format.
         *
         * The logical device is borrowed and must remain valid for the lifetime
         * of this object.
         *
         * Shader data and vertex input descriptions are borrowed only during
         * construction and are not retained by the graphics pipeline.
         *
         * @param device Logical device used to create and destroy pipeline
         * resources.
         * @param color_attachment_format Format of the color attachment used
         * with the graphics pipeline.
         * @param graphics_shader_data Borrowed SPIR-V bytecode for the vertex
         * and fragment shader stages.
         * @param graphics_vertex_input_data Borrowed vertex input binding and
         * attribute descriptions.
         * @param graphics_pipeline_layout_data Borrowed pipeline-layout
         * requirements, including push-constant ranges.
         *
         * @pre device must be a valid Vulkan logical-device handle.
         * @pre color_attachment_format must not be VK_FORMAT_UNDEFINED.
         * @pre graphics_shader_data.vertex_data must contain valid, nonempty
         * vertex shader SPIR-V.
         * @pre graphics_shader_data.fragment_data must contain valid, nonempty
         * fragment shader SPIR-V.
         *
         * @post Successful construction produces a non-null Vulkan pipeline
         * layout and graphics pipeline.
         *
         * @throws Core::Error::EngineError if shader-module creation,
         * pipeline-layout creation, or graphics-pipeline creation fails.
         */
        VulkanGraphicsPipeline(
            VkDevice device, VkFormat color_attachment_format,
            const GraphicsShaderData &graphics_shader_data,
            const GraphicsVertexInputData &graphics_vertex_input_data,
            const GraphicsPipelineLayoutData &graphics_pipeline_layout_data);

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
} // namespace SNE::Engine::Renderer::Vulkan
