#pragma once

#include "engine/renderer/shader/push_constant_range.hpp"
#include "engine/renderer/shader/shader_descriptor_binding.hpp"
#include "engine/renderer/shader/shader_entry_point_selection.hpp"
#include <vector>

namespace SNE::Engine::Renderer::Shader {
    /**
     * @brief Describes the resource interface of a shader entry point.
     *
     * Associates a selected shader entry point with metadata describing
     * the resources accessed by that entry point.
     *
     * Stores push-constant ranges and descriptor binding requirements
     * associated with the selected shader entry point.
     *
     * The description owns its metadata and is independent of Vulkan
     * shader modules, graphics pipelines, and GPU resource ownership.
     */
    struct ShaderInterface {
        /**
         * @brief Shader execution stage and exported entry-point name.
         *
         * Identifies the shader entry point associated with the
         * reflected interface metadata.
         */
        ShaderEntryPointSelection entry_point{};

        /**
         * @brief Push-constant byte ranges accessed by the entry point.
         *
         * Contains the offset, size, and execution stage of each
         * represented push-constant range.
         *
         * An empty vector indicates that no push-constant ranges
         * are represented by this interface.
         */
        std::vector<PushConstantRange> push_constant_ranges;

        /**
         * @brief Descriptor bindings required by the shader entry point.
         *
         * Contains the descriptor set, binding index, resource type,
         * and descriptor count for each required resource.
         *
         * An empty vector indicates that the entry point does not
         * require any descriptor bindings.
         */
        std::vector<ShaderDescriptorBinding> descriptor_bindings;
    };
} // namespace SNE::Engine::Renderer::Shader
