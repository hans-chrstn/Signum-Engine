#pragma once

#include "engine/core/math/matrix4.hpp"
#include "engine/core/math/quaternion.hpp"
#include "engine/core/math/vector3.hpp"
#include "engine/platform/input/cursor_position.hpp"

namespace SNE::Editor {
    namespace Platform = SNE::Engine::Platform;
    namespace Math = SNE::Engine::Core::Math;

    /**
     * @brief Manages camera navigation state for the editor's Scene View.
     *
     * Maintains the camera orientation, orbit target, and distance
     * used to construct a world-to-camera view transformation.
     *
     * Tracks mouse-drag interaction state for viewport navigation.
     *
     * Uses the engine's mathematical primitives without depending
     * on Dear ImGui or Vulkan.
     *
     * The controller does not own rendering resources or manage
     * the engine's active rendering camera.
     */
    class SceneViewCameraController {
      private:
        /**
         * @brief Default distance between the camera and its orbit target.
         */
        static constexpr float kDefaultOrbitDistance = 3.0F;

        /**
         * @brief Whether an orbit drag interaction is currently active.
         */
        bool m_IsDragging{false};

        /**
         * @brief Previous cursor position used to calculate drag movement.
         *
         * Coordinates are expressed in logical window units.
         */
        Platform::CursorPosition m_PreviousCursor{};

        /**
         * @brief Camera orientation relative to its orbit target.
         *
         * Initialized to the identity rotation and maintained as a
         * unit quaternion during camera navigation.
         */
        Math::Quaternionf m_Orientation{Math::quaternionIdentity()};

        /**
         * @brief World-space position around which the camera orbits.
         *
         * Initially located at the world origin.
         */
        Math::Vector3f m_OrbitTarget{};

        /**
         * @brief Distance between the camera and its orbit target.
         *
         * Initialized to kDefaultOrbitDistance.
         *
         * Must remain finite and greater than zero during navigation.
         */
        float m_OrbitDistance{kDefaultOrbitDistance};

      public:
        /**
         * @brief Constructs a Scene View camera controller.
         *
         * Initializes the camera at its default orbit distance
         * from the world origin with an identity orientation.
         *
         * No graphics or platform resources are allocated.
         */
        SceneViewCameraController() = default;

        /**
         * @brief Destroys the Scene View camera controller.
         *
         * No explicit resource cleanup is required.
         */
        ~SceneViewCameraController() = default;

        /**
         * @brief Computes the current world-to-camera view matrix.
         *
         * Constructs the camera's world transformation from its orbit
         * target, orientation, and distance, then inverts the result.
         *
         * Does not modify camera state or rendering resources.
         *
         * @return View matrix transforming world-space coordinates
         * into camera space.
         *
         * @pre The camera orientation must represent a valid rotation.
         * @pre The orbit target and distance must contain finite values.
         *
         * @note Triggers an invariant assertion if the camera's
         * world transformation cannot be inverted.
         */
        [[nodiscard]] auto viewMatrix() const -> Math::Matrix4f;

        /**
         * @brief Rotates the Scene View camera around its orbit target.
         *
         * Applies yaw and pitch rotations to the current camera orientation.
         * Both angles are expressed in radians.
         *
         * Positive angles follow the engine's right-handed rotation
         * convention.
         *
         * The resulting orientation is normalized to maintain a
         * valid rotation quaternion.
         *
         * Does not modify the orbit target, orbit distance, or
         * renderer state.
         *
         * @param yaw_radians Rotation around the camera's local Y axis.
         * @param pitch_radians Rotation around the camera's local X axis.
         *
         * @pre Both rotation angles must be finite.
         *
         * @post The camera orientation remains normalized.
         */
        auto orbit(float yaw_radians, float pitch_radians) -> void;

        /**
         * @brief Begins an orbit drag interaction.
         *
         * Records the initial cursor position and activates orbit
         * dragging. An existing drag is restarted from the supplied
         * cursor position.
         *
         * Does not change the camera orientation or view matrix.
         *
         * @param cursor_position Initial cursor position in logical
         * window coordinates.
         *
         * @pre Both cursor coordinates must be finite.
         */
        auto beginOrbitDrag(Platform::CursorPosition cursor_position) -> void;

        /**
         * @brief Ends the current orbit drag interaction.
         *
         * Deactivates orbit dragging without modifying the camera
         * orientation, orbit target, or distance.
         *
         * Has no effect when no drag is active.
         */
        auto endOrbitDrag() noexcept -> void;

        /**
         * @brief Updates the camera orientation during an active orbit drag.
         *
         * Calculates cursor movement relative to the previous cursor
         * position and converts the movement into yaw and pitch angles.
         *
         * Does not modify the camera when orbit dragging is inactive.
         *
         * Updates the previous cursor position after successfully
         * processing an active drag.
         *
         * @param cursor_position Current cursor position in logical
         * window coordinates.
         * @param radians_per_unit Rotation sensitivity in radians per
         * logical cursor coordinate unit.
         *
         * @return true if camera orientation was updated; otherwise false.
         *
         * @pre Cursor coordinates must be finite.
         * @pre radians_per_unit must be finite and greater than zero.
         * @pre Computed rotation angles must be finite.
         */
        [[nodiscard]] auto
        updateOrbitDrag(Platform::CursorPosition cursor_position,
                        float radians_per_unit) -> bool;

        /**
         * @brief Computes the camera's current world-space position.
         *
         * Rotates the camera's local positive-Z distance offset by
         * its orientation, then adds the world-space orbit target.
         *
         * Does not modify camera state or rendering resources.
         *
         * @return Camera position in world coordinates.
         *
         * @pre The camera orientation must represent a valid rotation.
         * @pre The orbit target and distance must contain finite values.
         */
        [[nodiscard]] auto cameraWorldPosition() const -> Math::Vector3f;
    };
} // namespace SNE::Editor
