#include "vulkan_graphics_pipeline.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/core/numeric/checked_conversion.hpp"
#include "engine/renderer/pipeline/blend_factor.hpp"
#include "engine/renderer/pipeline/blend_operation.hpp"
#include "engine/renderer/pipeline/color_write_mask.hpp"
#include "engine/renderer/pipeline/compare_op.hpp"
#include "engine/renderer/pipeline/cull_mode.hpp"
#include "engine/renderer/pipeline/front_face.hpp"
#include "engine/renderer/pipeline/primitive_topology.hpp"
#include "engine/renderer/shader/graphics_shader_data.hpp"
#include "engine/renderer/shader/shader_interface.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include "engine/renderer/vulkan/common/vulkan_result.hpp"
#include "engine/renderer/vulkan/descriptor/vulkan_descriptor_set_layout.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_descriptor_compatibility.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_push_constant_compatibility.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_spirv_reflection.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_spirv_validation.hpp"
#include "vulkan_shader_module.hpp"
#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace Renderer = SNE::Engine::Renderer;
namespace Core = SNE::Engine::Core;

namespace {
    [[nodiscard]] auto toVulkanCullMode(Renderer::CullMode mode)
        -> VkCullModeFlags {
        switch (mode) {
        case Renderer::CullMode::None:
            return VK_CULL_MODE_NONE;
        case Renderer::CullMode::Front:
            return VK_CULL_MODE_FRONT_BIT;
        case Renderer::CullMode::Back:
            return VK_CULL_MODE_BACK_BIT;
        }

        Core::Assertion::failAssertion(
            Core::Assertion::AssertionType::Precondition,
            Core::Error::Subsystem::Vulkan,
            "Vulkan cull mode conversion requires a valid CullMode value");
    }

    [[nodiscard]] auto toVulkanFrontFace(Renderer::FrontFace front_face)
        -> VkFrontFace {
        switch (front_face) {
        case Renderer::FrontFace::Clockwise:
            return VK_FRONT_FACE_CLOCKWISE;
        case Renderer::FrontFace::CounterClockwise:
            return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        }

        Core::Assertion::failAssertion(
            Core::Assertion::AssertionType::Precondition,
            Core::Error::Subsystem::Vulkan,
            "Vulkan front-face conversion requires a valid FrontFace value");
    }

    [[nodiscard]] auto
    toVulkanPrimitiveTopology(Renderer::PrimitiveTopology topology)
        -> VkPrimitiveTopology {
        switch (topology) {
        case Renderer::PrimitiveTopology::PointList:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case Renderer::PrimitiveTopology::LineList:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case Renderer::PrimitiveTopology::LineStrip:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case Renderer::PrimitiveTopology::TriangleList:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case Renderer::PrimitiveTopology::TriangleStrip:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        }

        Core::Assertion::failAssertion(
            Core::Assertion::AssertionType::Precondition,
            Core::Error::Subsystem::Vulkan,
            "Vulkan primitive topology conversion requires a valid "
            "PrimitiveTopology value");
    }

    [[nodiscard]] auto toVulkanCompareOp(Renderer::CompareOp operation)
        -> VkCompareOp {
        switch (operation) {
        case Renderer::CompareOp::Never:
            return VK_COMPARE_OP_NEVER;
        case Renderer::CompareOp::Less:
            return VK_COMPARE_OP_LESS;
        case Renderer::CompareOp::Equal:
            return VK_COMPARE_OP_EQUAL;
        case Renderer::CompareOp::LessOrEqual:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case Renderer::CompareOp::Greater:
            return VK_COMPARE_OP_GREATER;
        case Renderer::CompareOp::NotEqual:
            return VK_COMPARE_OP_NOT_EQUAL;
        case Renderer::CompareOp::GreaterOrEqual:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case Renderer::CompareOp::Always:
            return VK_COMPARE_OP_ALWAYS;
        }

        Core::Assertion::failAssertion(
            Core::Assertion::AssertionType::Precondition,
            Core::Error::Subsystem::Vulkan,
            "Vulkan comparison conversion requires a valid CompareOp value");
    }

    [[nodiscard]] auto toVulkanBlendFactor(Renderer::BlendFactor factor)
        -> VkBlendFactor {
        switch (factor) {
        case Renderer::BlendFactor::Zero:
            return VK_BLEND_FACTOR_ZERO;
        case Renderer::BlendFactor::One:
            return VK_BLEND_FACTOR_ONE;
        case Renderer::BlendFactor::SrcColor:
            return VK_BLEND_FACTOR_SRC_COLOR;
        case Renderer::BlendFactor::OneMinusSrcColor:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case Renderer::BlendFactor::DstColor:
            return VK_BLEND_FACTOR_DST_COLOR;
        case Renderer::BlendFactor::OneMinusDstColor:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case Renderer::BlendFactor::SrcAlpha:
            return VK_BLEND_FACTOR_SRC_ALPHA;
        case Renderer::BlendFactor::OneMinusSrcAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case Renderer::BlendFactor::DstAlpha:
            return VK_BLEND_FACTOR_DST_ALPHA;
        case Renderer::BlendFactor::OneMinusDstAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        }

        Core::Assertion::failAssertion(
            Core::Assertion::AssertionType::Precondition,
            Core::Error::Subsystem::Vulkan,
            "Vulkan blend factor conversion requires a valid BlendFactor "
            "value");
    }

    [[nodiscard]] auto
    toVulkanBlendOperation(Renderer::BlendOperation operation) -> VkBlendOp {
        switch (operation) {
        case Renderer::BlendOperation::Add:
            return VK_BLEND_OP_ADD;
        case Renderer::BlendOperation::Subtract:
            return VK_BLEND_OP_SUBTRACT;
        case Renderer::BlendOperation::ReverseSubtract:
            return VK_BLEND_OP_REVERSE_SUBTRACT;
        case Renderer::BlendOperation::Min:
            return VK_BLEND_OP_MIN;
        case Renderer::BlendOperation::Max:
            return VK_BLEND_OP_MAX;
        }

        Core::Assertion::failAssertion(
            Core::Assertion::AssertionType::Precondition,
            Core::Error::Subsystem::Vulkan,
            "Vulkan blend operation conversion requires a valid BlendOperation "
            "value");
    }

    [[nodiscard]] auto
    toVulkanColorWriteMask(const Renderer::ColorWriteMask &mask)
        -> VkColorComponentFlags {
        VkColorComponentFlags flags{0U};

        if (mask.red) {
            flags |= VK_COLOR_COMPONENT_R_BIT;
        }

        if (mask.green) {
            flags |= VK_COLOR_COMPONENT_G_BIT;
        }

        if (mask.blue) {
            flags |= VK_COLOR_COMPONENT_B_BIT;
        }

        if (mask.alpha) {
            flags |= VK_COLOR_COMPONENT_A_BIT;
        }

        return flags;
    }
} // namespace

namespace SNE::Engine::Renderer::Vulkan {
    auto validateGraphicsShaderData(const Shader::GraphicsShaderData &data)
        -> void {
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
        VkDevice device, GraphicsAttachmentFormats attachment_formats,
        const Shader::GraphicsShaderData &graphics_shader_data,
        const GraphicsVertexInputData &graphics_vertex_input_data,
        const GraphicsPipelineLayoutData &graphics_pipeline_layout_data,
        std::uint32_t max_push_constants_size, GraphicsPipelineState state)
        : m_Device(device) {
        if (device == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanGraphicsPipeline requires a valid Vulkan device");
        }

        if (attachment_formats.color == VK_FORMAT_UNDEFINED) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanGraphicsPipeline requires a valid color attachment "
                "format");
        }

        if (attachment_formats.depth == VK_FORMAT_UNDEFINED) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanGraphicsPipeline requires a valid depth attachment "
                "format");
        }

        validateGraphicsShaderData(graphics_shader_data);
        validateSpirvEntryPoint(graphics_shader_data.vertex.artifact.words,
                                graphics_shader_data.vertex.selection);
        validateSpirvEntryPoint(graphics_shader_data.fragment.artifact.words,
                                graphics_shader_data.fragment.selection);
        const Shader::ShaderInterface vertex_interface =
            reflectSpirvInterface(graphics_shader_data.vertex.artifact.words,
                                  graphics_shader_data.vertex.selection);
        const Shader::ShaderInterface fragment_interface =
            reflectSpirvInterface(graphics_shader_data.fragment.artifact.words,
                                  graphics_shader_data.fragment.selection);
        validatePushConstantCompatibility(
            vertex_interface,
            graphics_pipeline_layout_data.push_constant_ranges,
            max_push_constants_size);
        validatePushConstantCompatibility(
            fragment_interface,
            graphics_pipeline_layout_data.push_constant_ranges,
            max_push_constants_size);

        std::vector<VkDescriptorSetLayout> layouts;
        std::vector<DescriptorSetLayoutBindingView>
            descriptor_set_layout_binding_views;

        const auto &borrowed_layouts =
            graphics_pipeline_layout_data.descriptor_set_layouts;

        layouts.reserve(borrowed_layouts.size());
        descriptor_set_layout_binding_views.reserve(borrowed_layouts.size());

        for (const VulkanDescriptorSetLayout *layout : borrowed_layouts) {
            if (layout == nullptr) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanGraphicsPipeline requires non-null descriptor-set "
                    "layout references");
            }

            const VkDescriptorSetLayout handle = layout->nativeHandle();

            if (handle == VK_NULL_HANDLE) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanGraphicsPipeline requires valid descriptor-set "
                    "layout handles");
            }

            layouts.push_back(handle);
            descriptor_set_layout_binding_views.push_back(
                DescriptorSetLayoutBindingView{
                    .bindings = layout->bindingDescriptions(),
                });
        }

        validateDescriptorCompatibility(vertex_interface,
                                        descriptor_set_layout_binding_views);

        validateDescriptorCompatibility(fragment_interface,
                                        descriptor_set_layout_binding_views);

        VulkanShaderModule vertex_shader = VulkanShaderModule(
            m_Device, graphics_shader_data.vertex.artifact.words);
        VulkanShaderModule fragment_shader = VulkanShaderModule(
            m_Device, graphics_shader_data.fragment.artifact.words);

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
        input_assembly_state_create_info.topology =
            toVulkanPrimitiveTopology(state.topology);
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
        raster_state_create_info.cullMode = toVulkanCullMode(state.cull_mode);
        raster_state_create_info.frontFace =
            toVulkanFrontFace(state.front_face);
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

        VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info{};
        depth_stencil_state_create_info.sType =
            VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil_state_create_info.depthTestEnable =
            state.depth_test_enabled ? VK_TRUE : VK_FALSE;
        depth_stencil_state_create_info.depthWriteEnable =
            state.depth_write_enabled ? VK_TRUE : VK_FALSE;
        depth_stencil_state_create_info.depthCompareOp =
            toVulkanCompareOp(state.depth_compare);
        depth_stencil_state_create_info.depthBoundsTestEnable = VK_FALSE;
        depth_stencil_state_create_info.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState color_blend_attachment_state{};
        color_blend_attachment_state.blendEnable =
            state.color_blend.enabled ? VK_TRUE : VK_FALSE;
        color_blend_attachment_state.srcColorBlendFactor =
            toVulkanBlendFactor(state.color_blend.source_color_factor);
        color_blend_attachment_state.dstColorBlendFactor =
            toVulkanBlendFactor(state.color_blend.destination_color_factor);
        color_blend_attachment_state.colorBlendOp =
            toVulkanBlendOperation(state.color_blend.color_operation);
        color_blend_attachment_state.srcAlphaBlendFactor =
            toVulkanBlendFactor(state.color_blend.source_alpha_factor);
        color_blend_attachment_state.dstAlphaBlendFactor =
            toVulkanBlendFactor(state.color_blend.destination_alpha_factor);
        color_blend_attachment_state.alphaBlendOp =
            toVulkanBlendOperation(state.color_blend.alpha_operation);
        color_blend_attachment_state.colorWriteMask =
            toVulkanColorWriteMask(state.color_blend.write_mask);

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
            &attachment_formats.color;
        rendering_create_info.depthAttachmentFormat = attachment_formats.depth;

        VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
        pipeline_layout_create_info.sType =
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        const auto layout_count =
            Core::Numeric::tryConvertToUint32(layouts.size());

        if (!layout_count.has_value()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanGraphicsPipeline descriptor-set layout count exceeds "
                "uint32_t");
        }

        pipeline_layout_create_info.setLayoutCount = layout_count.value();
        pipeline_layout_create_info.pSetLayouts = layouts.data();

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
            graphics_pipeline_create_info.pDepthStencilState =
                &depth_stencil_state_create_info;
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
