#pragma once

#include "engine/renderer/shader/shader_stage.hpp"
#include <string>
namespace SNE::Engine::Renderer::Shader {
    /**
     * @brief Describes the selected entry point of a compiled shader.
     *
     * Associates a shader execution stage with the exported
     * entry-point name used by the compiled shader artifact.
     *
     * The description is independent of graphics APIs and owns
     * its entry-point name.
     *
     * Consumers are responsible for validating that the specified
     * entry point exists and supports the requested shader stage.
     */
    struct ShaderEntryPointSelection {
        /**
         * @brief Shader execution stage associated with the entry point.
         */
        ShaderStage stage{};

        /**
         * @brief Exported entry-point name within the compiled shader.
         *
         * The name must be nonempty when used for shader execution.
         * Default initialization produces an empty string.
         */
        std::string entry_point;
    };
} // namespace SNE::Engine::Renderer::Shader
