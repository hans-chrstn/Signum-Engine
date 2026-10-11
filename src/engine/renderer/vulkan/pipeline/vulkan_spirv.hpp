#pragma once

#include "engine/renderer/shader/shader_artifact.hpp"
#include <filesystem>

namespace Shader = SNE::Engine::Renderer::Shader;
namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Loads compiled SPIR-V bytecode into an owning shader artifact.
     *
     * Reads a binary file containing SPIR-V bytecode and stores its
     * contents as 32-bit words in a ShaderArtifact.
     *
     * The returned artifact owns its bytecode storage independently
     * of the source file and Vulkan GPU resources.
     *
     * Validates file accessibility, byte size, and successful reading.
     * Does not validate SPIR-V instruction contents or entry points.
     *
     * @param path Path to the compiled SPIR-V file.
     * @return Shader artifact owning the loaded SPIR-V words.
     *
     * @throws Core::Error::EngineError if the file cannot be opened,
     * has an invalid byte size, or cannot be read completely.
     */
    [[nodiscard]] auto loadSpirv(const std::filesystem::path &path)
        -> Shader::ShaderArtifact;
} // namespace SNE::Engine::Renderer::Vulkan
