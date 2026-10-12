#pragma once

#include "engine/core/math/matrix4.hpp"

namespace SNE::Engine::Renderer {
    /**
     * @brief Describes the initial camera configuration for rendering.
     *
     * Stores a view transformation and perspective projection parameters
     * independently of renderer-specific graphics APIs.
     *
     * The view matrix transforms world-space coordinates into camera
     * space and defaults to the identity transformation.
     *
     * The rendering backend derives the projection aspect ratio from
     * the current drawable framebuffer dimensions.
     *
     * Projection parameters must be configured with valid values before
     * perspective projection is constructed.
     */
    struct InitialCameraData {
        /**
         * @brief World-to-camera transformation matrix.
         *
         * Defaults to the identity matrix, representing a camera
         * positioned at the world origin without rotation.
         */
        Core::Math::Matrix4f view{Core::Math::identity()};

        /**
         * @brief Vertical field of view expressed in radians.
         *
         * Must be finite, greater than zero, and less than pi.
         */
        float fov_y_radians{};

        /**
         * @brief Positive distance to the near clipping plane.
         *
         * Must be finite, greater than zero, and less than far_plane.
         */
        float near_plane{};

        /**
         * @brief Distance to the far clipping plane.
         *
         * Must be finite and greater than near_plane.
         */
        float far_plane{};
    };
} // namespace SNE::Engine::Renderer
