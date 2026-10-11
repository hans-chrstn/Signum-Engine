#pragma once

#include "engine/renderer/shader/shader_stage.hpp"
#include <cstdint>

namespace SNE::Engine::Renderer::Shader {
    /**
     * @brief Describes a shader-declared push-constant byte range.
     *
     * Identifies the offset, size, and shader execution stage associated
     * with a push-constant range.
     *
     * The description is independent of graphics APIs and does not own
     * GPU resources.
     *
     * @note Offsets and sizes are expressed in bytes.
     */
    struct PushConstantRange {
        /**
         * @brief Byte offset of the push-constant range.
         */
        std::uint32_t offset{};

        /**
         * @brief Size of the push-constant range in bytes.
         */
        std::uint32_t size{};

        /**
         * @brief Shader execution stage that accesses the range.
         */
        ShaderStage stage{};
    };
} // namespace SNE::Engine::Renderer::Shader
