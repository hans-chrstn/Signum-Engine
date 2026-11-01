#pragma once

#include "scene_view/scene_grid_geometry.hpp"
#include <array>
#include <cstdint>
#include <span>
#include <vector>

namespace SNE::Editor {
    /**
     * @brief Represents a vertex used by the editor's ground grid.
     *
     * Stores a three-dimensional position in world space.
     *
     * The vertex representation is independent of Vulkan and
     * contains only the attributes required for grid rendering.
     */
    struct SceneGridVertex {
        /**
         * @brief World-space vertex position in X, Y, Z order.
         */
        std::array<float, 3U> position{};
    };

    /**
     * @brief Owns CPU-side mesh geometry for the editor's ground grid.
     *
     * Stores vertices and indices representing independent line segments.
     *
     * Each consecutive pair of indices identifies the two endpoints
     * of one line segment.
     *
     * The geometry is independent of graphics APIs and can be
     * consumed by the engine's existing mesh upload infrastructure.
     *
     * @note Indices use 16-bit storage to match the current mesh
     * upload contract.
     * @note This structure does not own GPU resources.
     */
    struct SceneGridMesh {
        /**
         * @brief Contiguous vertex storage for the grid.
         */
        std::vector<SceneGridVertex> vertices;

        /**
         * @brief Indices referencing vertices in pairs to form lines.
         *
         * Every index must reference an existing vertex.
         * Each consecutive pair defines one independent line segment.
         */
        std::vector<std::uint16_t> indices;
    };

    /**
     * @brief Converts editor grid line segments into indexed mesh geometry.
     *
     * Creates two vertices for each input line, preserving the world-space
     * positions of its starting and ending endpoints.
     *
     * Generates consecutive 16-bit indices that reference each vertex pair
     * as an independent line segment.
     *
     * The resulting geometry is intended for rendering with
     * PrimitiveTopology::LineList.
     *
     * @param lines Borrowed grid line segments to convert.
     *
     * @return CPU-owned vertices and indices representing the grid lines.
     *
     * @pre The total vertex count must be representable using 16-bit indices.
     *
     * @post The resulting mesh contains two vertices and two indices
     * for every input line.
     *
     * @note An empty input produces an empty mesh.
     * @note This function does not allocate GPU resources or perform rendering.
     *
     * @throws std::bad_alloc If CPU geometry allocation fails.
     */
    [[nodiscard]] auto makeSceneGridMesh(std::span<const SceneGridLine> lines)
        -> SceneGridMesh;
} // namespace SNE::Editor
