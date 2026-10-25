#pragma once

#include "engine/renderer/shader/shader_source_language.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include <filesystem>
#include <string>

namespace SNE::Engine::Renderer::Shader {
    /**
     * @brief Describes a shader source to be compiled.
     *
     * Identifies the source file, programming language, execution stage,
     * and source-level entry point required for shader compilation.
     *
     * Owns its filesystem path and entry-point name independently of
     * the caller's storage.
     *
     * This description is independent of graphics APIs and does not
     * own compiled shader artifacts or GPU resources.
     *
     * Consumers are responsible for validating the source path,
     * language, stage, and entry point before compilation.
     */
    struct ShaderSourceDescription {
        /**
         * @brief Filesystem path to the shader source file.
         *
         * The path may be relative or absolute. Its interpretation
         * depends on the consuming compilation operation.
         *
         * An empty path is not valid for file-based compilation.
         */
        std::filesystem::path path{};

        /**
         * @brief Programming language used by the shader source.
         *
         * Determines which compiler implementation processes the source.
         * Defaults to Slang.
         */
        ShaderSourceLanguage language{ShaderSourceLanguage::Slang};

        /**
         * @brief Shader execution stage to compile.
         *
         * Identifies the vertex, fragment, or compute stage requested
         * from the source compiler.
         */
        ShaderStage stage{};

        /**
         * @brief Name of the entry point declared in the source.
         *
         * Identifies the source-level function selected for compilation.
         * The compiler may export a different entry-point name in the
         * resulting shader artifact.
         *
         * The name must be nonempty when compiling a shader.
         */
        std::string entry_point;
    };
} // namespace SNE::Engine::Renderer::Shader
