#pragma once

#include <cstdint>
namespace SNE::Engine::Renderer {
    /**
     * @brief Defines which triangle winding order is considered front-facing.
     */
    enum class FrontFace : std::uint8_t {
        /** @brief Treats clockwise-wound triangles as front-facing. */
        Clockwise,

        /** @brief Treats counterclockwise-wound triangles as front-facing. */
        CounterClockwise,
    };
} // namespace SNE::Engine::Renderer
