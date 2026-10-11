
#pragma once

#include "engine/renderer/shader/shader_entry_point_selection.hpp"
#include "engine/renderer/shader/shader_interface.hpp"
#include <cstdint>
#include <span>

namespace Shader = SNE::Engine::Renderer::Shader;

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Reflects the resource interface of a compiled SPIR-V shader.
     *
     * Extracts shader interface metadata for the specified execution
     * stage and exported entry-point name.
     *
     * The resulting interface owns its metadata and is independent
     * of Vulkan resource ownership and the underlying reflection library.
     *
     * The SPIR-V bytecode and entry-point selection are borrowed and
     * remain owned by the caller.
     *
     * Reflection does not create Vulkan GPU resources.
     *
     * @param words Borrowed compiled SPIR-V module represented as 32-bit words.
     * @param selection Shader execution stage and exported entry-point name.
     *
     * @return Owned shader interface metadata for the selected entry point.
     *
     * @throws Core::Error::EngineError if the SPIR-V module is invalid,
     * the requested entry point is unavailable, or reflection fails.
     */
    [[nodiscard]] auto
    reflectSpirvInterface(std::span<const std::uint32_t> words,
                          const Shader::ShaderEntryPointSelection &selection)
        -> Shader::ShaderInterface;
} // namespace SNE::Engine::Renderer::Vulkan
