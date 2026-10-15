#pragma once

#include "engine/renderer/pipeline/graphics_pipeline_state.hpp"
#include "engine/renderer/shader/graphics_shader_data.hpp"

namespace SNE::Engine::Renderer {
    /**
     * @brief Describes the rendering configuration of a material.
     *
     * Combines shader inputs and graphics pipeline state to define
     * how geometry using this material should be rendered.
     *
     * Owns its shader configuration but does not own GPU resources.
     *
     * Rendering backends use this description to select or create
     * compatible graphics pipelines.
     */
    struct MaterialDescription {
        /**
         * @brief Vertex and fragment shaders used by the material.
         *
         * Owns the compiled shader bytecode and selected entry points.
         */
        Shader::GraphicsShaderData shaders{};

        /**
         * @brief Graphics pipeline configuration used by the material.
         *
         * Controls depth testing, culling, blending, and other
         * supported rendering states.
         */
        GraphicsPipelineState pipeline_state{};

        /**
         * @brief Compares two material descriptions.
         *
         * @param other Material description to compare.
         *
         * @return True when shader configurations and pipeline states match.
         */
        [[nodiscard]] constexpr auto
        operator==(const MaterialDescription &other) const -> bool = default;
    };
} // namespace SNE::Engine::Renderer
