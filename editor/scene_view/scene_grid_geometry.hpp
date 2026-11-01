#pragma once

#include "engine/core/math/vector3.hpp"
#include <cstdint>
#include <vector>

namespace SNE::Editor {
    namespace Math = SNE::Engine::Core::Math;

    /**
     * @brief Represents a line segment in the editor's world-space grid.
     *
     * Stores two endpoints in world coordinates.
     * Grid lines lie on the XZ plane with Y equal to zero.
     *
     * Does not own rendering or GPU resources.
     */
    struct SceneGridLine {
        /**
         * @brief Starting endpoint of the grid line in world space.
         */
        Math::Vector3f start{};

        /**
         * @brief Ending endpoint of the grid line in world space.
         */
        Math::Vector3f end{};
    };

    /**
     * @brief Configures the geometry of the editor's finite ground grid.
     *
     * Defines the grid's extent and spacing in world units.
     * The grid is centered at the origin on the XZ plane.
     */
    struct SceneGridParameters {
        /**
         * @brief Number of cells extending from the origin in each direction.
         *
         * Must be greater than zero.
         */
        std::uint32_t half_extent_cells{};

        /**
         * @brief Distance between adjacent grid lines in world units.
         *
         * Must be finite and greater than zero.
         */
        float grid_spacing{};
    };

    /**
     * @brief Generates line segments for a finite editor ground grid.
     *
     * Produces evenly spaced lines parallel to the X and Z axes,
     * centered at the world origin on the XZ plane.
     *
     * @param parameters Grid half-extent and spacing configuration.
     *
     * @return CPU-owned line segments describing the grid.
     *
     * @pre parameters.half_extent_cells must be greater than zero.
     * @pre parameters.grid_spacing must be finite and greater than zero.
     *
     * @note This function does not allocate GPU resources or
     * perform any rendering operations.
     */
    [[nodiscard]] auto generateSceneGridLines(SceneGridParameters parameters)
        -> std::vector<SceneGridLine>;
} // namespace SNE::Editor
