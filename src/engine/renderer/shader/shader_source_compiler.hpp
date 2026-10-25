#pragma once

#include "engine/renderer/shader/shader_compilation_result.hpp"
#include "engine/renderer/shader/shader_source_description.hpp"
namespace SNE::Engine::Renderer::Shader {
    /**
     * @brief Compiles a shader source into a renderer-compatible artifact.
     *
     * Selects the appropriate shader compiler according to the supplied
     * source language and produces compiled shader bytecode with its
     * exported entry-point selection.
     *
     * The returned result owns its compiled artifact and any diagnostics
     * produced during successful compilation.
     *
     * Compilation does not create Vulkan shader modules, graphics
     * pipelines, or GPU resources.
     *
     * @param source Shader source file and compilation entry-point selection.
     *
     * @return Compiled shader stage input and retained diagnostics.
     *
     * @throws Core::Error::EngineError if source loading, compiler
     * initialization, or shader compilation fails.
     * @throws std::bad_alloc if required host allocation fails.
     */
    [[nodiscard]] auto
    compileShaderSource(const ShaderSourceDescription &source)
        -> ShaderCompilationResult;
} // namespace SNE::Engine::Renderer::Shader
