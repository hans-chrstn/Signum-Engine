#include "vulkan_graphics_pipeline_cache.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_graphics_pipeline_variant_key.hpp"
#include <utility>

namespace SNE::Engine::Renderer::Vulkan {
    auto VulkanGraphicsPipelineCache::find(
        const VulkanGraphicsPipelineVariantKey &key) const noexcept
        -> const VulkanGraphicsPipeline * {
        for (const CacheEntry &entry : m_Entries) {
            if (entry.key == key) {
                return &entry.pipeline;
            }
        }
        return nullptr;
    }

    auto VulkanGraphicsPipelineCache::getOrCreate(
        VkDevice device, GraphicsAttachmentFormats attachment_formats,
        const Shader::GraphicsShaderData &graphics_shader_data,
        const GraphicsVertexInputData &graphics_vertex_input_data,
        const GraphicsPipelineLayoutData &graphics_pipeline_layout_data,
        std::uint32_t max_push_constants_size, GraphicsPipelineState state)
        -> const VulkanGraphicsPipeline * {
        VulkanGraphicsPipelineVariantKey key =
            makeVulkanGraphicsPipelineVariantKey(
                device, attachment_formats, graphics_shader_data,
                graphics_vertex_input_data, graphics_pipeline_layout_data,
                state);

        const VulkanGraphicsPipeline *existing = find(key);

        if (existing != nullptr) {
            return existing;
        }

        VulkanGraphicsPipeline pipeline{device,
                                        attachment_formats,
                                        graphics_shader_data,
                                        graphics_vertex_input_data,
                                        graphics_pipeline_layout_data,
                                        max_push_constants_size,
                                        state};

        CacheEntry entry{
            .key = std::move(key),
            .pipeline = std::move(pipeline),
        };

        m_Entries.emplace_back(std::move(entry));

        return &m_Entries.back().pipeline;
    }
} // namespace SNE::Engine::Renderer::Vulkan
