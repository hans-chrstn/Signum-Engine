#pragma once

#include <cstdint>
#include <vector>
namespace SNE::Engine::Renderer::Shader {
    /**
     * @brief Owns compiled SPIR-V shader bytecode.
     *
     * Stores compiled shader instructions as a sequence of 32-bit words.
     *
     * The artifact owns its bytecode storage and is independent of
     * Vulkan shader modules, graphics pipelines, and GPU resource
     * ownership.
     */
    struct ShaderArtifact {
        /**
         * @brief Compiled SPIR-V bytecode represented as 32-bit words.
         *
         * Each element occupies four bytes. The vector size represents
         * the number of words rather than the total byte count.
         */
        std::vector<std::uint32_t> words;

        /**
         * @brief Compares two compiled shader artifacts.
         *
         * @param other Shader artifact to compare.
         *
         * @return True when both artifacts contain identical SPIR-V words.
         */
        [[nodiscard]] constexpr auto
        operator==(const ShaderArtifact &other) const -> bool = default;
    };
} // namespace SNE::Engine::Renderer::Shader
