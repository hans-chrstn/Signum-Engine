#pragma once

#include "engine/renderer/shader/shader_stage_input.hpp"
#include <string>
namespace SNE::Engine::Renderer::Shader {
    /**
     * @brief Contains the output of a successful shader compilation.
     *
     * Owns the compiled shader stage input and any compiler diagnostics
     * produced during compilation.
     *
     * Diagnostics may contain warnings or informational messages even
     * when compilation succeeds.
     *
     * This structure is independent of shader compiler implementations
     * and graphics APIs.
     */
    struct ShaderCompilationResult {
        /**
         * @brief Compiled shader artifact and exported entry-point selection.
         */
        ShaderStageInput stage_input;

        /**
         * @brief Compiler diagnostics retained from successful compilation.
         *
         * An empty string indicates that no diagnostics were reported.
         */
        std::string diagnostics;
    };
} // namespace SNE::Engine::Renderer::Shader
