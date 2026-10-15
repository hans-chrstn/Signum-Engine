#pragma once

#include "engine/renderer/shader/shader_stage_input.hpp"

namespace SNE::Engine::Renderer::Shader {
    /**
     * @brief Owns the vertex and fragment shaders used by a graphics pipeline.
     *
     * Contains compiled shader bytecode and entry-point selections for
     * the vertex and fragment stages.
     *
     * Each shader stage owns its SPIR-V bytecode and entry-point selection.
     * The data remains available for graphics-pipeline creation and
     * recreation.
     *
     * This description is independent of graphics APIs and does not
     * own GPU resources.
     */
    struct GraphicsShaderData {
        /**
         * @brief Vertex shader used during graphics-pipeline creation.
         *
         * Contains compiled SPIR-V bytecode and the selected vertex
         * entry point.
         */
        ShaderStageInput vertex{};

        /**
         * @brief Fragment shader used during graphics-pipeline creation.
         *
         * Contains compiled SPIR-V bytecode and the selected fragment
         * entry point.
         */
        ShaderStageInput fragment{};

        /**
         * @brief Compares two graphics shader configurations.
         *
         * @param other Graphics shader configuration to compare.
         *
         * @return True when both vertex and fragment shader inputs
         * are equal.
         */
        [[nodiscard]] constexpr auto
        operator==(const GraphicsShaderData &other) const -> bool = default;
    };
} // namespace SNE::Engine::Renderer::Shader
