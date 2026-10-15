#pragma once

#include <cstdint>
namespace SNE::Engine::Renderer {
    /**
     * @brief Defines how vertices are assembled into primitives.
     */
    enum class PrimitiveTopology : std::uint8_t {
        /** @brief Treats each vertex as an independent point. */
        PointList,

        /** @brief Assembles each pair of vertices into an independent line. */
        LineList,

        /** @brief Assembles consecutive vertices into connected lines. */
        LineStrip,

        /** @brief Assembles each group of three vertices into an independent
           triangle. */
        TriangleList,

        /** @brief Assembles consecutive vertices into connected triangles. */
        TriangleStrip,
    };
} // namespace SNE::Engine::Renderer
