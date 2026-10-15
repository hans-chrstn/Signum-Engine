#pragma once

#include <cstdint>
namespace SNE::Engine::Renderer {
    /**
     * @brief Specifies which triangle faces are discarded during rasterization.
     */
    enum class CullMode : std::uint8_t {
        /** @brief Disables face culling. */
        None,

        /** @brief Discards front-facing triangles. */
        Front,

        /** @brief Discards back-facing triangles. */
        Back,
    };
} // namespace SNE::Engine::Renderer
