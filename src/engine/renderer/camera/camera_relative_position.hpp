#pragma once

#include "engine/core/math/vector3.hpp"
#include "engine/core/math/vector3d.hpp"
#include <optional>

namespace SNE::Engine::Renderer {
    /**
     * @brief Positions used to calculate camera-relative coordinates.
     *
     * Both positions must use the same coordinate system,
     * reference frame, and distance units.
     */
    struct CameraRelativePositionInput {
        /** World-space position to convert. */
        Core::Math::Vector3d world_position{};

        /** World-space position of the camera. */
        Core::Math::Vector3d camera_position{};
    };

    /**
     * @brief Converts a world-space position to camera-relative coordinates.
     *
     * Subtracts the camera position from the world position using double
     * precision before converting the result to single precision.
     *
     * Preserves small relative offsets between large world-space coordinates
     * that could otherwise be lost through premature float conversion.
     *
     * Both positions must use the same coordinate system, reference frame,
     * and distance units.
     *
     * @param world_position World-space position of the object.
     * @param camera_position World-space position of the camera.
     *
     * @return Camera-relative position in single precision, or std::nullopt
     * if either input contains non-finite coordinates, the subtraction
     * produces non-finite results, or any relative coordinate exceeds the
     * finite representable range of float.
     *
     * @note The conversion applies translation only. Camera orientation,
     * scaling, and projection are not applied.
     *
     * @note Valid conversions may lose precision when the resulting
     * double-precision coordinates are rounded to float.
     *
     * @note This function is independent of Vulkan and GPU resource ownership.
     */
    [[nodiscard]] auto
    tryMakeCameraRelativePosition(CameraRelativePositionInput input) noexcept
        -> std::optional<Core::Math::Vector3f>;
} // namespace SNE::Engine::Renderer
