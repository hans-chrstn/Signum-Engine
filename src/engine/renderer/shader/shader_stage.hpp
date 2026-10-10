#pragma once

#include <cstdint>
namespace SNE::Engine::Renderer::Shader {
    /**
     * @brief Identifies a shader execution stage.
     *
     * Represents shader stages independently of graphics APIs.
     * Backend implementations translate these values into their
     * corresponding native shader-stage representations.
     */
    enum class ShaderStage : std::uint8_t {
        /**
         * @brief Processes vertex inputs and produces vertex outputs.
         */
        Vertex,

        /**
         * @brief Processes fragments and produces fragment outputs.
         */
        Fragment,

        /**
         * @brief Executes general-purpose GPU compute workloads.
         */
        Compute,
    };
} // namespace SNE::Engine::Renderer::Shader
