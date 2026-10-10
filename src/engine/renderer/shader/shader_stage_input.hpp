#pragma once

#include "engine/renderer/shader/shader_entry_point_selection.hpp"
#include <cstdint>
#include <span>
namespace SNE::Engine::Renderer::Shader {
    /**
     * @brief Describes borrowed SPIR-V bytecode and its selected shader entry
     * point.
     *
     * Provides a non-owning view of compiled SPIR-V words together with
     * the shader execution stage and exported entry-point name.
     *
     * The referenced SPIR-V storage must remain valid throughout
     * any operation that consumes this description.
     *
     * This type is independent of Vulkan resource ownership.
     */
    struct ShaderStageInput {
        /**
         * @brief Borrowed view of compiled SPIR-V bytecode.
         *
         * Contains 32-bit SPIR-V words without copying or owning
         * the underlying storage.
         *
         * The owning shader artifact must remain valid while this
         * view is accessed.
         */
        std::span<const std::uint32_t> words;

        /**
         * @brief Selected execution stage and exported entry-point name.
         *
         * The consumer must validate that the entry point exists
         * in the supplied SPIR-V and matches the requested stage.
         */
        ShaderEntryPointSelection selection{};
    };
} // namespace SNE::Engine::Renderer::Shader
