#pragma once

#include <cstdint>

namespace SNE::Engine::Renderer::Shader {
    /**
     * @brief Identifies the source language of a shader.
     *
     * Determines which shader compiler is used to translate source code
     * into a compiled shader artifact.
     *
     * The language selection is independent of graphics APIs and shader
     * execution stages.
     */
    enum class ShaderSourceLanguage : std::uint8_t {
        /** Shader source written in Slang. */
        Slang,

        /** Shader source written in HLSL. */
        Hlsl,

        /** Shader source written in GLSL. */
        Glsl,
    };
} // namespace SNE::Engine::Renderer::Shader
