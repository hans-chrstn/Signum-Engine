#pragma once

#include "engine/renderer/shader/shader_entry_point_selection.hpp"
#include <cstdint>
#include <span>

namespace Shader = SNE::Engine::Renderer::Shader;

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Validates a compiled SPIR-V module for the Vulkan backend.
     *
     * Validates the supplied SPIR-V instructions against the Vulkan 1.4
     * target environment using SPIRV-Tools.
     *
     * The bytecode is borrowed and remains owned by the caller.
     * Validation does not modify the bytecode or create GPU resources.
     *
     * This operation validates the SPIR-V module, not whether a
     * separately requested entry-point name and execution stage match.
     *
     * @param words Borrowed SPIR-V module represented as 32-bit words.
     *
     * @throws Core::Error::EngineError if the module fails validation.
     */
    auto validateSpirvModule(std::span<const std::uint32_t> words) -> void;

    /**
     * @brief Validates a requested entry point in a compiled SPIR-V module.
     *
     * Validates the supplied SPIR-V module against the Vulkan 1.4 target
     * environment before inspecting its declared entry points.
     *
     * Verifies that the module contains an entry point matching both the
     * requested exported name and shader execution stage.
     *
     * The bytecode and selection are borrowed and remain owned by the caller.
     * Validation does not modify the bytecode or create GPU resources.
     *
     * @param words Borrowed SPIR-V module represented as 32-bit words.
     * @param selection Requested shader stage and exported entry-point name.
     *
     * @throws Core::Error::EngineError if the SPIR-V module is invalid,
     * parsing fails, or the requested entry point and stage are unavailable.
     */
    auto
    validateSpirvEntryPoint(std::span<const std::uint32_t> words,
                            const Shader::ShaderEntryPointSelection &selection)
        -> void;
} // namespace SNE::Engine::Renderer::Vulkan
