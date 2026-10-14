#pragma once

#include "engine/renderer/shader/shader_artifact.hpp"
#include "engine/renderer/shader/shader_entry_point_selection.hpp"

namespace SNE::Engine::Renderer::Shader {
    /**
     * @brief Owns compiled shader bytecode and its selected entry point.
     *
     * Stores a compiled shader artifact alongside the execution stage
     * and exported entry-point name used to execute it.
     *
     * The shader bytecode and entry-point selection are owned by this
     * description and remain available for subsequent pipeline creation
     * or recreation.
     *
     * This type is independent of graphics APIs and does not own
     * GPU resources.
     */
    struct ShaderStageInput {
        /**
         * @brief Compiled shader bytecode owned by this stage.
         */
        ShaderArtifact artifact;

        /**
         * @brief Selected execution stage and exported entry-point name.
         *
         * The consumer must validate that the entry point exists
         * in the supplied shader artifact and matches the requested stage.
         */
        ShaderEntryPointSelection selection{};
    };
} // namespace SNE::Engine::Renderer::Shader
