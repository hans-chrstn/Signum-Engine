#pragma once

#include <cstdint>
#include <span>

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
} // namespace SNE::Engine::Renderer::Vulkan
