#pragma once

#include "engine/renderer/vulkan/pipeline/vulkan_graphics_pipeline.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_graphics_pipeline_variant_key.hpp"
#include <deque>
namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Owns and reuses Vulkan graphics pipelines by configuration.
     *
     * Stores graphics pipelines alongside their variant keys to avoid
     * creating duplicate pipelines for identical configurations.
     *
     * Cached pipelines are owned exclusively by this class and released
     * through RAII when their entries are destroyed.
     *
     * Pipeline lookups return non-owning pointers. The cache must outlive
     * any use of those pointers.
     *
     * @note The cache must not destroy pipelines while pending GPU
     * submissions may still reference them.
     *
     * @note Descriptor-set layouts referenced by cached pipelines must
     * remain valid for the required Vulkan resource lifetime.
     */
    class VulkanGraphicsPipelineCache {
      private:
        /**
         * @brief Associates a pipeline configuration with its Vulkan pipeline.
         *
         * Owns the graphics pipeline and stores the configuration used
         * to identify it during cache lookups.
         *
         * The variant key stores owned configuration data but does not
         * own Vulkan device or descriptor-set layout resources.
         */
        struct CacheEntry {
            /**
             * @brief Configuration identifying the cached graphics pipeline.
             *
             * Used to compare requested pipeline configurations against
             * existing cache entries.
             */
            VulkanGraphicsPipelineVariantKey key;

            /**
             * @brief Vulkan graphics pipeline owned by this cache entry.
             *
             * Its lifetime is managed through VulkanGraphicsPipeline RAII.
             */
            VulkanGraphicsPipeline pipeline;
        };

        /**
         * @brief Collection of owned graphics pipeline cache entries.
         *
         * Appending entries preserves references and pointers to
         * existing elements.
         *
         * Removing an entry invalidates references to that entry and
         * destroys its owned Vulkan pipeline.
         */
        std::deque<CacheEntry> m_Entries;

      public:
        /**
         * @brief Finds a cached graphics pipeline matching a variant key.
         *
         * @param key Pipeline configuration to find.
         *
         * @return Pointer to the matching pipeline, or nullptr if absent.
         *
         * @note The returned pointer is non-owning and remains valid
         * only while the cache retains the corresponding entry.
         */
        [[nodiscard]] auto
        find(const VulkanGraphicsPipelineVariantKey &key) const noexcept
            -> const VulkanGraphicsPipeline *;

        /**
         * @brief Finds or creates a graphics pipeline for a configuration.
         *
         * Reuses a cached pipeline when an identical variant exists.
         * Otherwise, creates and stores a new Vulkan graphics pipeline.
         *
         * @param device Logical device used for pipeline creation.
         * @param attachment_formats Required attachment formats.
         * @param graphics_shader_data Vertex and fragment shader inputs.
         * @param graphics_vertex_input_data Borrowed vertex descriptions.
         * @param graphics_pipeline_layout_data Borrowed pipeline layout inputs.
         * @param max_push_constants_size Physical device push-constant limit.
         * @param state Graphics pipeline configuration.
         *
         * @return Non-owning pointer to the cached graphics pipeline.
         *
         * @throws Core::Error::EngineError if pipeline creation fails.
         *
         * @note Cached pipelines must not be destroyed while in use by the GPU.
         */
        [[nodiscard]] auto getOrCreate(
            VkDevice device, GraphicsAttachmentFormats attachment_formats,
            const Shader::GraphicsShaderData &graphics_shader_data,
            const GraphicsVertexInputData &graphics_vertex_input_data,
            const GraphicsPipelineLayoutData &graphics_pipeline_layout_data,
            std::uint32_t max_push_constants_size,
            GraphicsPipelineState state = {}) -> const VulkanGraphicsPipeline *;
    };
} // namespace SNE::Engine::Renderer::Vulkan
