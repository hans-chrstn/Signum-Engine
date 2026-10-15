#include "vulkan_graphics_pipeline_variant_key.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/vulkan/descriptor/vulkan_descriptor_set_layout.hpp"

namespace SNE::Engine::Renderer::Vulkan {
    auto makeVulkanGraphicsPipelineVariantKey(
        VkDevice device, GraphicsAttachmentFormats attachment_formats,
        const Shader::GraphicsShaderData &graphics_shader_data,
        const GraphicsVertexInputData &graphics_vertex_input_data,
        const GraphicsPipelineLayoutData &graphics_pipeline_layout_data,
        GraphicsPipelineState state) -> VulkanGraphicsPipelineVariantKey {
        VulkanGraphicsPipelineVariantKey key{};
        key.device = device;
        key.shaders = graphics_shader_data;
        key.state = state;
        key.attachment_formats = attachment_formats;

        key.vertex_bindings.reserve(graphics_vertex_input_data.bindings.size());

        for (const VkVertexInputBindingDescription &binding :
             graphics_vertex_input_data.bindings) {
            key.vertex_bindings.push_back(VulkanVertexBindingKey{
                .binding = binding.binding,
                .stride = binding.stride,
                .input_rate = binding.inputRate,
            });
        }

        key.vertex_attributes.reserve(
            graphics_vertex_input_data.attributes.size());

        for (const VkVertexInputAttributeDescription &attribute :
             graphics_vertex_input_data.attributes) {
            key.vertex_attributes.push_back(VulkanVertexAttributeKey{
                .location = attribute.location,
                .binding = attribute.binding,
                .format = attribute.format,
                .offset = attribute.offset,
            });
        }

        key.push_constant_ranges.reserve(
            graphics_pipeline_layout_data.push_constant_ranges.size());

        for (const VkPushConstantRange &range :
             graphics_pipeline_layout_data.push_constant_ranges) {
            key.push_constant_ranges.push_back(VulkanPushConstantRangeKey{
                .stage_flags = range.stageFlags,
                .offset = range.offset,
                .size = range.size,
            });
        }

        key.descriptor_set_layouts.reserve(
            graphics_pipeline_layout_data.descriptor_set_layouts.size());

        for (const VulkanDescriptorSetLayout *layout :
             graphics_pipeline_layout_data.descriptor_set_layouts) {
            if (layout == nullptr) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "Pipeline variant key requires non-null "
                    "descriptor-set layout references");
            }

            const VkDescriptorSetLayout handle = layout->nativeHandle();

            if (handle == VK_NULL_HANDLE) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "Pipeline variant key requires valid "
                    "descriptor-set layout handles");
            }

            key.descriptor_set_layouts.push_back(handle);
        }

        return key;
    }
} // namespace SNE::Engine::Renderer::Vulkan
