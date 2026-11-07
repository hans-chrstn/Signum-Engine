#include "scene_view_camera_controller.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/core/math/matrix4.hpp"
#include "engine/core/math/quaternion.hpp"
#include "engine/core/math/vector2.hpp"
#include "engine/core/math/vector3.hpp"
#include "engine/platform/input/cursor_position.hpp"
#include <cmath>

namespace SNE::Editor {
    namespace Core = Engine::Core;
    namespace Math = Core::Math;
    namespace Assertion = Core::Assertion;
    namespace Error = Core::Error;

    auto SceneViewCameraController::viewMatrix() const -> Math::Matrix4f {
        const Math::Matrix4f target_translation =
            Math::translate(m_OrbitTarget.x, m_OrbitTarget.y, m_OrbitTarget.z);

        const Math::Matrix4f target_rotation =
            Math::toRotationMatrix(m_Orientation);

        const Math::Matrix4f distance_translation =
            Math::translate(0.0F, 0.0F, m_OrbitDistance);

        const Math::Matrix4f matrix = Math::multiply(
            target_translation,
            Math::multiply(target_rotation, distance_translation));

        const std::optional<Math::Matrix4f> matrix_inverse =
            Math::tryInverse(matrix);

        if (!matrix_inverse.has_value()) {
            Assertion::failAssertion(
                Assertion::AssertionType::Invariant, Error::Subsystem::Editor,
                "Scene View camera world transform must be invertible");
        }

        return matrix_inverse.value();
    }

    auto SceneViewCameraController::orbit(float yaw_radians,
                                          float pitch_radians) -> void {
        if (!std::isfinite(yaw_radians) || !std::isfinite(pitch_radians)) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition,
                Error::Subsystem::Editor,
                "Scene View camera orbit angles must be finite");
        }

        const Math::Quaternionf yaw_quaternion =
            Math::fromAxisAngle({.x = 0.0F, .y = 1.0F, .z = 0.0F}, yaw_radians);

        const Math::Quaternionf pitch_quaternion = Math::fromAxisAngle(
            {.x = 1.0F, .y = 0.0F, .z = 0.0F}, pitch_radians);

        const Math::Quaternionf drag_quaternion =
            Math::multiply(yaw_quaternion, pitch_quaternion);

        m_Orientation =
            Math::normalize(Math::multiply(m_Orientation, drag_quaternion));
    }

    auto SceneViewCameraController::beginOrbitDrag(
        Platform::CursorPosition cursor_position) -> void {
        if (!std::isfinite(cursor_position.x) ||
            !std::isfinite(cursor_position.y)) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition,
                Error::Subsystem::Editor,
                "Scene View orbit cursor position must be finite");
        }

        m_PreviousCursor = cursor_position;

        m_DragMode = DragMode::Orbit;
    }

    auto SceneViewCameraController::endOrbitDrag() noexcept -> void {
        if (m_DragMode == DragMode::Orbit) {
            m_DragMode = DragMode::None;
        }
    }

    auto SceneViewCameraController::updateOrbitDrag(
        Platform::CursorPosition cursor_position, float radians_per_unit)
        -> bool {
        if (!std::isfinite(cursor_position.x) ||
            !std::isfinite(cursor_position.y)) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition,
                Error::Subsystem::Editor,
                "Scene View orbit cursor position must be finite");
        }

        if (!std::isfinite(radians_per_unit) || radians_per_unit <= 0.0F) {
            Assertion::failAssertion(Assertion::AssertionType::Precondition,
                                     Error::Subsystem::Editor,
                                     "Scene View orbit sensitivity must be "
                                     "finite and greater than zero");
        }

        if (m_DragMode != DragMode::Orbit) {
            return false;
        }

        const Platform::CursorPosition cursor_delta{
            .x = cursor_position.x - m_PreviousCursor.x,
            .y = cursor_position.y - m_PreviousCursor.y,
        };

        if (cursor_delta.x == 0.0 && cursor_delta.y == 0.0) {
            return false;
        }

        const auto yaw = static_cast<float>(-cursor_delta.x * radians_per_unit);
        const auto pitch =
            static_cast<float>(-cursor_delta.y * radians_per_unit);

        if (!std::isfinite(yaw) || !std::isfinite(pitch)) {
            Assertion::failAssertion(Assertion::AssertionType::Precondition,
                                     Error::Subsystem::Editor,
                                     "Scene View orbit cursor movement "
                                     "produces invalid rotation angles");
        }

        orbit(yaw, pitch);

        m_PreviousCursor = cursor_position;

        return true;
    }

    auto SceneViewCameraController::cameraWorldPosition() const
        -> Math::Vector3f {
        const Math::Vector3f local_offset{
            .x = 0.0F,
            .y = 0.0F,
            .z = m_OrbitDistance,
        };

        const Math::Matrix4f rotation_matrix{
            Math::toRotationMatrix(m_Orientation),
        };

        const Math::Vector3f rotated_offset{
            Math::transformDirection(rotation_matrix, local_offset),
        };

        return Math::add(m_OrbitTarget, rotated_offset);
    }

    auto SceneViewCameraController::pan(Math::Vector3f displacement) -> void {
        if (!std::isfinite(displacement.x) || !std::isfinite(displacement.y) ||
            !std::isfinite(displacement.z)) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition,
                Error::Subsystem::Editor,
                "Scene View pan displacement must be finite");
        }

        const Math::Vector3f new_target =
            Math::add(m_OrbitTarget, displacement);

        if (!std::isfinite(new_target.x) || !std::isfinite(new_target.y) ||
            !std::isfinite(new_target.z)) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition,
                Error::Subsystem::Editor,
                "Scene View pan produces invalid orbit target");
        }

        m_OrbitTarget = new_target;
    }

    auto SceneViewCameraController::panCameraPlane(Math::Vector2f displacement)
        -> void {
        if (!std::isfinite(displacement.x) || !std::isfinite(displacement.y)) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition,
                Error::Subsystem::Editor,
                "Scene View camera-relative pan components must be finite");
        }

        const Math::Vector3f local_displacement{
            .x = displacement.x,
            .y = displacement.y,
            .z = 0.0F,
        };

        const Math::Matrix4f rotation_matrix{
            Math::toRotationMatrix(m_Orientation),
        };

        const Math::Vector3f world_displacement =
            Math::transformDirection(rotation_matrix, local_displacement);

        pan(world_displacement);
    }

    auto SceneViewCameraController::beginPanDrag(
        Platform::CursorPosition cursor_position) -> void {
        if (!std::isfinite(cursor_position.x) ||
            !std::isfinite(cursor_position.y)) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition,
                Error::Subsystem::Editor,
                "Scene View pan cursor position must be finite");
        }

        m_PreviousCursor = cursor_position;

        m_DragMode = DragMode::Pan;
    }

    auto SceneViewCameraController::endPanDrag() noexcept -> void {
        if (m_DragMode == DragMode::Pan) {
            m_DragMode = DragMode::None;
        }
    }

    auto SceneViewCameraController::updatePanDrag(
        Platform::CursorPosition cursor_position,
        float world_units_per_cursor_unit) -> bool {
        if (!std::isfinite(cursor_position.x) ||
            !std::isfinite(cursor_position.y)) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition,
                Error::Subsystem::Editor,
                "Scene View pan cursor position must be finite");
        }

        if (!std::isfinite(world_units_per_cursor_unit) ||
            world_units_per_cursor_unit <= 0.0F) {
            Assertion::failAssertion(Assertion::AssertionType::Precondition,
                                     Error::Subsystem::Editor,
                                     "Scene View pan sensitivity must be "
                                     "finite and greater than zero");
        }

        if (m_DragMode != DragMode::Pan) {
            return false;
        }

        const Platform::CursorPosition cursor_delta{
            .x = cursor_position.x - m_PreviousCursor.x,
            .y = cursor_position.y - m_PreviousCursor.y,
        };

        if (cursor_delta.x == 0.0 && cursor_delta.y == 0.0) {
            return false;
        }

        const Math::Vector2f camera_displacement{
            .x = static_cast<float>(-cursor_delta.x) *
                 world_units_per_cursor_unit,
            .y = static_cast<float>(cursor_delta.y) *
                 world_units_per_cursor_unit,
        };

        if (!std::isfinite(camera_displacement.x) ||
            !std::isfinite(camera_displacement.y)) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition,
                Error::Subsystem::Editor,
                "Scene View pan cursor movement produces invalid displacement");
        }

        panCameraPlane(camera_displacement);

        m_PreviousCursor = cursor_position;

        return true;
    }
} // namespace SNE::Editor
