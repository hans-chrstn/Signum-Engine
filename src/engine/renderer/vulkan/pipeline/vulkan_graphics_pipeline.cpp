#include "vulkan_graphics_pipeline.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/core/numeric/checked_conversion.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include "engine/renderer/vulkan/common/vulkan_result.hpp"
#include "vulkan_shader_module.hpp"
#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>

namespace SNE::Engine::Renderer::Vulkan {
    auto validateGraphicsShaderData(const GraphicsShaderData &data) -> void {
        if (data.vertex.selection.stage != Shader::ShaderStage::Vertex) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanGraphicsPipeline requires the vertex shader input "
                "to specify ShaderStage::Vertex");
        }

        if (data.fragment.selection.stage != Shader::ShaderStage::Fragment) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanGraphicsPipeline requires the fragment shader input "
                "to specify ShaderStage::Fragment");
        }

        if (data.vertex.selection.entry_point.empty()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanGraphicsPipeline requires a nonempty vertex shader "
                "entry-point name");
        }

        if (data.fragment.selection.entry_point.empty()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanGraphicsPipeline requires a nonempty fragment shader "
                "entry-point name");
        }
    }

    VulkanGraphicsPipeline::VulkanGraphicsPipeline(
        VkDevice device, VkFormat color_attachment_format,
        const GraphicsShaderData &graphics_shader_data,
        const GraphicsVertexInputData &graphics_vertex_input_data,
        const GraphicsPipelineLayoutData &graphics_pipeline_layout_data)
        : m_Device(device) {
        if (device == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanGraphicsPipeline requires a valid Vulkan device");
        }

        if (color_attachment_format == VK_FORMAT_UNDEFINED) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanGraphicsPipeline requires a valid color attachment "
                "format");
        }

        validateGraphicsShaderData(graphics_shader_data);

        VulkanShaderModule vertex_shader =
            VulkanShaderModule(m_Device, graphics_shader_data.vertex.words);
        VulkanShaderModule fragment_shader =
            VulkanShaderModule(m_Device, graphics_shader_data.fragment.words);

        VkPipelineShaderStageCreateInfo vertex_stage_description{};
        vertex_stage_description.sType =
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        // runs during the vertex stage
        vertex_stage_description.stage = VK_SHADER_STAGE_VERTEX_BIT;
        // Actual compiled vertex shader
        vertex_stage_description.module = vertex_shader.nativeHandle();
        // Shader function Vulkan starts executing
        vertex_stage_description.pName =
            graphics_shader_data.vertex.selection.entry_point.c_str();

        VkPipelineShaderStageCreateInfo fragment_stage_description{};
        fragment_stage_description.sType =
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        // runs during the fragment stage
        fragment_stage_description.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        // Actual compiled fragment shader
        fragment_stage_description.module = fragment_shader.nativeHandle();
        // Shader function Vulkan starts executing
        fragment_stage_description.pName =
            graphics_shader_data.fragment.selection.entry_point.c_str();

        VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
        vertex_input_state_create_info.sType =
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        const std::optional<std::uint32_t> binding_count =
            Core::Numeric::tryConvertToUint32(
                graphics_vertex_input_data.bindings.size());
        if (!binding_count.has_value()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanGraphicsPipeline vertex binding count exceeds uint32_t");
        }
        vertex_input_state_create_info.vertexBindingDescriptionCount =
            binding_count.value();
        vertex_input_state_create_info.pVertexBindingDescriptions =
            graphics_vertex_input_data.bindings.data();

        const std::optional<std::uint32_t> attribute_count =
            Core::Numeric::tryConvertToUint32(
                graphics_vertex_input_data.attributes.size());
        if (!attribute_count.has_value()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanGraphicsPipeline vertex attribute count exceeds "
                "uint32_t");
        }
        vertex_input_state_create_info.vertexAttributeDescriptionCount =
            attribute_count.value();
        vertex_input_state_create_info.pVertexAttributeDescriptions =
            graphics_vertex_input_data.attributes.data();

        VkPipelineInputAssemblyStateCreateInfo
            input_assembly_state_create_info{};
        input_assembly_state_create_info.sType =
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        // Every 3 vertices form an independent triangle
        input_assembly_state_create_info.topology =
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        // Don't use special restart indices to break primitive strips
        input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewport_state_create_info{};
        viewport_state_create_info.sType =
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        // Pipeline uses one viewport.
        viewport_state_create_info.viewportCount = 1U;
        viewport_state_create_info.pViewports = nullptr;
        // Pipeline uses one scissor rectangle
        viewport_state_create_info.scissorCount = 1U;
        viewport_state_create_info.pScissors = nullptr;

        VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
        dynamic_state_create_info.sType =
            VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        // We have two dynamic states: viewport and scissor.
        dynamic_state_create_info.dynamicStateCount = 2U;

        const std::array<VkDynamicState, 2> dynamic_states{
            {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR},
        };
        dynamic_state_create_info.pDynamicStates = dynamic_states.data();

        VkPipelineRasterizationStateCreateInfo raster_state_create_info{};
        raster_state_create_info.sType =
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        raster_state_create_info.depthClampEnable = VK_FALSE;
        // Don't discard geometry before rasterization
        raster_state_create_info.rasterizerDiscardEnable = VK_FALSE;
        // Fill triangle interiors normally
        raster_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
        // Don't throw away front-facing or back-facing triangles
        raster_state_create_info.cullMode = VK_CULL_MODE_NONE;
        // Counter-clockwise vertex order is considered front-facing
        raster_state_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        // Don't offset fragment depth values
        raster_state_create_info.depthBiasEnable = VK_FALSE;
        // Normal line width
        raster_state_create_info.lineWidth = 1.0F;

        VkPipelineMultisampleStateCreateInfo multisample_state_create_info{};
        multisample_state_create_info.sType =
            VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        // One sample per pixel, basically no msaa
        multisample_state_create_info.rasterizationSamples =
            VK_SAMPLE_COUNT_1_BIT;
        // Don't run the fragment shader independently for multiple samples
        multisample_state_create_info.sampleShadingEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState color_blend_attachment_state{};
        // Fragment shader output replaces the existing color directly
        color_blend_attachment_state.blendEnable = VK_FALSE;
        // Allow the pipeline to write all RGBA channels
        color_blend_attachment_state.colorWriteMask =
            static_cast<VkColorComponentFlags>(VK_COLOR_COMPONENT_R_BIT) |
            static_cast<VkColorComponentFlags>(VK_COLOR_COMPONENT_G_BIT) |
            static_cast<VkColorComponentFlags>(VK_COLOR_COMPONENT_B_BIT) |
            static_cast<VkColorComponentFlags>(VK_COLOR_COMPONENT_A_BIT);

        VkPipelineColorBlendStateCreateInfo color_blend_state_create_info{};
        color_blend_state_create_info.sType =
            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        // Don't perform bitwise logic operations between new and existing
        // colors
        color_blend_state_create_info.logicOpEnable = VK_FALSE;
        // We currently render into one color attachment
        color_blend_state_create_info.attachmentCount = 1U;
        color_blend_state_create_info.pAttachments =
            &color_blend_attachment_state;

        VkPipelineRenderingCreateInfo rendering_create_info{};
        rendering_create_info.sType =
            VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        // This pipeline renders into one color attachment
        rendering_create_info.colorAttachmentCount = 1U;
        rendering_create_info.pColorAttachmentFormats =
            &color_attachment_format;

        VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
        pipeline_layout_create_info.sType =
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        // No descriptor-set layouts yet
        pipeline_layout_create_info.setLayoutCount = 0U;
        // No descriptor-set layout array because count is zero
        pipeline_layout_create_info.pSetLayouts = nullptr;

        const auto push_constant_range_count =
            Core::Numeric::tryConvertToUint32(
                graphics_pipeline_layout_data.push_constant_ranges.size());
        if (!push_constant_range_count.has_value()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanGraphicsPipeline push-constant range count exceeds "
                "uint32_t");
        }
        pipeline_layout_create_info.pushConstantRangeCount =
            push_constant_range_count.value();
        pipeline_layout_create_info.pPushConstantRanges =
            graphics_pipeline_layout_data.push_constant_ranges.data();

        const VkResult pipeline_layout_result = vkCreatePipelineLayout(
            m_Device, &pipeline_layout_create_info, nullptr, &m_PipelineLayout);
        if (pipeline_layout_result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanPipelineLayoutCreationFailed,
                "Failed to create Vulkan pipeline layout",
                Core::Error::NativeError(
                    static_cast<int>(pipeline_layout_result),
                    std::string(toString(pipeline_layout_result))),
                "Create Vulkan Pipeline Layout");
        }

        try {
            if (m_PipelineLayout == VK_NULL_HANDLE) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Postcondition,
                    Core::Error::Subsystem::Vulkan,
                    "Successful Vulkan pipeline-layout creation must produce a "
                    "non-null pipeline layout handle");
            }

            VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{};
            graphics_pipeline_create_info.sType =
                VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            graphics_pipeline_create_info.pNext = &rendering_create_info;
            graphics_pipeline_create_info.stageCount = 2U;
            const std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages{
                {vertex_stage_description, fragment_stage_description},
            };
            graphics_pipeline_create_info.pStages = shader_stages.data();
            graphics_pipeline_create_info.pVertexInputState =
                &vertex_input_state_create_info;
            graphics_pipeline_create_info.pInputAssemblyState =
                &input_assembly_state_create_info;
            graphics_pipeline_create_info.pViewportState =
                &viewport_state_create_info;
            graphics_pipeline_create_info.pRasterizationState =
                &raster_state_create_info;
            graphics_pipeline_create_info.pMultisampleState =
                &multisample_state_create_info;
            graphics_pipeline_create_info.pDepthStencilState = nullptr;
            graphics_pipeline_create_info.pColorBlendState =
                &color_blend_state_create_info;
            graphics_pipeline_create_info.pDynamicState =
                &dynamic_state_create_info;
            graphics_pipeline_create_info.layout = m_PipelineLayout;
            graphics_pipeline_create_info.renderPass = VK_NULL_HANDLE;
            graphics_pipeline_create_info.subpass = 0U;
            graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
            graphics_pipeline_create_info.basePipelineIndex = -1;

            const VkResult result = vkCreateGraphicsPipelines(
                m_Device, VK_NULL_HANDLE, 1U, &graphics_pipeline_create_info,
                nullptr, &m_Pipeline);

            if (result != VK_SUCCESS) {
                throw Core::Error::EngineError(
                    Core::Error::Code::VulkanGraphicsPipelineCreationFailed,
                    "Failed to create Vulkan graphics pipeline",
                    Core::Error::NativeError(static_cast<int>(result),
                                             std::string(toString(result))),
                    "Create Vulkan Graphics Pipeline");
            }

            if (m_Pipeline == VK_NULL_HANDLE) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Postcondition,
                    Core::Error::Subsystem::Vulkan,
                    "Successful Vulkan graphics-pipeline creation must produce "
                    "a "
                    "non-null graphics pipeline handle");
            }
        } catch (...) {
            destroy();
            throw;
        }
    }

    auto VulkanGraphicsPipeline::destroy() noexcept -> void {
        if (m_Pipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
        }

        m_Pipeline = VK_NULL_HANDLE;

        if (m_PipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
        }

        m_PipelineLayout = VK_NULL_HANDLE;
        m_Device = VK_NULL_HANDLE;
    }

    VulkanGraphicsPipeline::~VulkanGraphicsPipeline() noexcept {
        destroy();
    }

    VulkanGraphicsPipeline::VulkanGraphicsPipeline(
        VulkanGraphicsPipeline &&other) noexcept
        : m_Device(std::exchange(other.m_Device, VK_NULL_HANDLE)),
          m_PipelineLayout(
              std::exchange(other.m_PipelineLayout, VK_NULL_HANDLE)),
          m_Pipeline(std::exchange(other.m_Pipeline, VK_NULL_HANDLE)) {}

    auto
    VulkanGraphicsPipeline::operator=(VulkanGraphicsPipeline &&other) noexcept
        -> VulkanGraphicsPipeline & {
        if (this == &other) {
            return *this;
        }

        destroy();

        m_Device = std::exchange(other.m_Device, VK_NULL_HANDLE);
        m_PipelineLayout =
            std::exchange(other.m_PipelineLayout, VK_NULL_HANDLE);
        m_Pipeline = std::exchange(other.m_Pipeline, VK_NULL_HANDLE);

        return *this;
    }

    auto VulkanGraphicsPipeline::nativeHandle() const noexcept -> VkPipeline {
        return m_Pipeline;
    }

    auto VulkanGraphicsPipeline::layoutHandle() const noexcept
        -> VkPipelineLayout {
        return m_PipelineLayout;
    }
} // namespace SNE::Engine::Renderer::Vulkan
