#include "engine/core/math/matrix4.hpp"
#include "scene_view/scene_view_camera_controller.hpp"
#include <cstddef>
#include <gtest/gtest.h>
#include <limits>
#include <numbers>

namespace Editor = SNE::Editor;
namespace Math = SNE::Engine::Core::Math;
namespace Platform = SNE::Engine::Platform;

namespace {
    constexpr float kTolerance = 1.0e-5F;
    constexpr float kDefaultDistance = 3.0F;
    constexpr float kRightAngle = std::numbers::pi_v<float> / 2.0F;

    auto expectMatricesNear(const Math::Matrix4f &actual,
                            const Math::Matrix4f &expected) -> void {
        for (std::size_t row{}; row < Math::kMatrix4Dimension; ++row) {
            for (std::size_t column{}; column < Math::kMatrix4Dimension;
                 ++column) {
                SCOPED_TRACE(::testing::Message()
                             << "row=" << row << ", column=" << column);

                EXPECT_NEAR(actual.at(row, column), expected.at(row, column),
                            kTolerance);
            }
        }
    }

    auto expectVectorsNear(const Math::Vector3f &actual,
                           const Math::Vector3f &expected) -> void {
        EXPECT_NEAR(actual.x, expected.x, kTolerance);
        EXPECT_NEAR(actual.y, expected.y, kTolerance);
        EXPECT_NEAR(actual.z, expected.z, kTolerance);
    }
} // namespace

TEST(SceneViewCameraControllerTests, DefaultCameraProducesExpectedViewMatrix) {
    const Editor::SceneViewCameraController controller{};

    const Math::Matrix4f expected =
        Math::translate(0.0F, 0.0F, -kDefaultDistance);

    expectMatricesNear(controller.viewMatrix(), expected);
}

TEST(SceneViewCameraControllerTests, ZeroOrbitPreservesViewMatrix) {
    Editor::SceneViewCameraController controller{};

    const Math::Matrix4f original = controller.viewMatrix();

    controller.orbit(0.0F, 0.0F);

    expectMatricesNear(controller.viewMatrix(), original);
}

TEST(SceneViewCameraControllerTests, PositiveYawProducesExpectedViewMatrix) {
    Editor::SceneViewCameraController controller{};

    controller.orbit(kRightAngle, 0.0F);

    const Math::Matrix4f expected =
        Math::multiply(Math::translate(0.0F, 0.0F, -kDefaultDistance),
                       Math::rotateY(-kRightAngle));

    expectMatricesNear(controller.viewMatrix(), expected);
}

TEST(SceneViewCameraControllerTests, PositivePitchProducesExpectedViewMatrix) {
    Editor::SceneViewCameraController controller{};

    controller.orbit(0.0F, kRightAngle);

    const Math::Matrix4f expected =
        Math::multiply(Math::translate(0.0F, 0.0F, -kDefaultDistance),
                       Math::rotateX(-kRightAngle));

    expectMatricesNear(controller.viewMatrix(), expected);
}

TEST(SceneViewCameraControllerTests, ComposesYawAndPitchInCorrectOrder) {
    Editor::SceneViewCameraController controller{};

    constexpr float kYaw = 0.4F;
    constexpr float kPitch = 0.3F;

    controller.orbit(kYaw, kPitch);

    const Math::Matrix4f expected_rotation =
        Math::multiply(Math::rotateX(-kPitch), Math::rotateY(-kYaw));

    const Math::Matrix4f expected = Math::multiply(
        Math::translate(0.0F, 0.0F, -kDefaultDistance), expected_rotation);

    expectMatricesNear(controller.viewMatrix(), expected);
}

TEST(SceneViewCameraControllerTests, AccumulatesSuccessiveOrbitRotations) {
    Editor::SceneViewCameraController controller{};

    constexpr float kYaw = 0.4F;
    constexpr float kPitch = 0.3F;

    controller.orbit(kYaw, 0.0F);
    controller.orbit(0.0F, kPitch);

    const Math::Matrix4f expected_rotation =
        Math::multiply(Math::rotateX(-kPitch), Math::rotateY(-kYaw));

    const Math::Matrix4f expected = Math::multiply(
        Math::translate(0.0F, 0.0F, -kDefaultDistance), expected_rotation);

    expectMatricesNear(controller.viewMatrix(), expected);
}

TEST(SceneViewCameraControllerTests, FullRotationRestoresOriginalViewMatrix) {
    Editor::SceneViewCameraController controller{};

    const Math::Matrix4f original = controller.viewMatrix();

    for (int i = 0; i < 4; ++i) {
        controller.orbit(kRightAngle, 0.0F);
    }

    expectMatricesNear(controller.viewMatrix(), original);
}

TEST(SceneViewCameraControllerTests, RejectsNonFiniteOrbitAngles) {
    Editor::SceneViewCameraController controller{};

    constexpr auto kExpectedMessage =
        "Scene View camera orbit angles must be finite";

    const float infinity = std::numeric_limits<float>::infinity();
    const float nan = std::numeric_limits<float>::quiet_NaN();

    ASSERT_DEATH(controller.orbit(infinity, 0.0F), kExpectedMessage);
    ASSERT_DEATH(controller.orbit(0.0F, infinity), kExpectedMessage);
    ASSERT_DEATH(controller.orbit(nan, 0.0F), kExpectedMessage);
    ASSERT_DEATH(controller.orbit(0.0F, nan), kExpectedMessage);
}

TEST(SceneViewCameraControllerTests, OrbitDragLifecyclePreservesCameraView) {
    constexpr float kYawRadians = 0.4F;
    constexpr float kPitchRadians = -0.2F;

    constexpr Platform::CursorPosition kInitialCursor{
        .x = 120.0,
        .y = 240.0,
    };

    constexpr Platform::CursorPosition kRestartCursor{
        .x = 300.0,
        .y = 400.0,
    };

    Editor::SceneViewCameraController controller{};

    controller.orbit(kYawRadians, kPitchRadians);

    const Math::Matrix4f expected = controller.viewMatrix();

    controller.beginOrbitDrag(kInitialCursor);
    expectMatricesNear(controller.viewMatrix(), expected);

    controller.beginOrbitDrag(kRestartCursor);
    expectMatricesNear(controller.viewMatrix(), expected);

    controller.endOrbitDrag();
    expectMatricesNear(controller.viewMatrix(), expected);

    controller.endOrbitDrag();
    expectMatricesNear(controller.viewMatrix(), expected);
}

TEST(SceneViewCameraControllerTests, RejectsNonFiniteOrbitDragCursor) {
    Editor::SceneViewCameraController controller{};

    constexpr auto kExpectedMessage =
        "Scene View orbit cursor position must be finite";

    const double infinity = std::numeric_limits<double>::infinity();
    const double nan = std::numeric_limits<double>::quiet_NaN();

    const Platform::CursorPosition infinite_x{
        .x = infinity,
        .y = 0.0,
    };

    const Platform::CursorPosition infinite_y{
        .x = 0.0,
        .y = infinity,
    };

    const Platform::CursorPosition nan_x{
        .x = nan,
        .y = 0.0,
    };

    const Platform::CursorPosition nan_y{
        .x = 0.0,
        .y = nan,
    };

    ASSERT_DEATH(controller.beginOrbitDrag(infinite_x), kExpectedMessage);
    ASSERT_DEATH(controller.beginOrbitDrag(infinite_y), kExpectedMessage);
    ASSERT_DEATH(controller.beginOrbitDrag(nan_x), kExpectedMessage);
    ASSERT_DEATH(controller.beginOrbitDrag(nan_y), kExpectedMessage);
}

TEST(SceneViewCameraControllerTests, InactiveDragIgnoresCursorMovement) {
    constexpr float kSensitivity = 0.01F;
    constexpr Platform::CursorPosition kCursor{
        .x = 120.0,
        .y = 180.0,
    };

    Editor::SceneViewCameraController controller{};
    const Math::Matrix4f original = controller.viewMatrix();

    EXPECT_FALSE(controller.updateOrbitDrag(kCursor, kSensitivity));

    expectMatricesNear(controller.viewMatrix(), original);
}

TEST(SceneViewCameraControllerTests, StationaryCursorDoesNotRotate) {
    constexpr float kSensitivity = 0.01F;
    constexpr Platform::CursorPosition kCursor{
        .x = 100.0,
        .y = 200.0,
    };

    Editor::SceneViewCameraController controller{};
    controller.beginOrbitDrag(kCursor);

    const Math::Matrix4f original = controller.viewMatrix();

    EXPECT_FALSE(controller.updateOrbitDrag(kCursor, kSensitivity));

    expectMatricesNear(controller.viewMatrix(), original);
}

TEST(SceneViewCameraControllerTests, CursorMovementProducesExpectedOrbit) {
    constexpr float kSensitivity = 0.01F;
    constexpr float kExpectedYaw = -0.2F;
    constexpr float kExpectedPitch = 0.2F;

    constexpr Platform::CursorPosition kInitialCursor{
        .x = 100.0,
        .y = 200.0,
    };

    constexpr Platform::CursorPosition kMovedCursor{
        .x = 120.0,
        .y = 180.0,
    };

    Editor::SceneViewCameraController controller{};

    controller.beginOrbitDrag(kInitialCursor);

    ASSERT_TRUE(controller.updateOrbitDrag(kMovedCursor, kSensitivity));

    const Math::Matrix4f expected_rotation = Math::multiply(
        Math::rotateX(-kExpectedPitch), Math::rotateY(-kExpectedYaw));

    const Math::Matrix4f expected = Math::multiply(
        Math::translate(0.0F, 0.0F, -kDefaultDistance), expected_rotation);

    expectMatricesNear(controller.viewMatrix(), expected);
}

TEST(SceneViewCameraControllerTests, RepeatedMovementUsesPreviousCursor) {
    constexpr float kSensitivity = 0.01F;
    constexpr float kExpectedTotalYaw = -0.25F;

    constexpr Platform::CursorPosition kInitialCursor{
        .x = 100.0,
        .y = 200.0,
    };

    constexpr Platform::CursorPosition kFirstCursor{
        .x = 120.0,
        .y = 200.0,
    };

    constexpr Platform::CursorPosition kSecondCursor{
        .x = 125.0,
        .y = 200.0,
    };

    Editor::SceneViewCameraController controller{};

    controller.beginOrbitDrag(kInitialCursor);

    ASSERT_TRUE(controller.updateOrbitDrag(kFirstCursor, kSensitivity));
    ASSERT_TRUE(controller.updateOrbitDrag(kSecondCursor, kSensitivity));

    const Math::Matrix4f expected =
        Math::multiply(Math::translate(0.0F, 0.0F, -kDefaultDistance),
                       Math::rotateY(-kExpectedTotalYaw));

    expectMatricesNear(controller.viewMatrix(), expected);

    EXPECT_FALSE(controller.updateOrbitDrag(kSecondCursor, kSensitivity));
}

TEST(SceneViewCameraControllerTests, EndingDragStopsCameraRotation) {
    constexpr float kSensitivity = 0.01F;

    constexpr Platform::CursorPosition kInitialCursor{
        .x = 100.0,
        .y = 200.0,
    };

    constexpr Platform::CursorPosition kMovedCursor{
        .x = 120.0,
        .y = 200.0,
    };

    constexpr Platform::CursorPosition kReleasedCursor{
        .x = 300.0,
        .y = 400.0,
    };

    Editor::SceneViewCameraController controller{};

    controller.beginOrbitDrag(kInitialCursor);
    ASSERT_TRUE(controller.updateOrbitDrag(kMovedCursor, kSensitivity));

    const Math::Matrix4f view_before_release = controller.viewMatrix();

    controller.endOrbitDrag();

    EXPECT_FALSE(controller.updateOrbitDrag(kReleasedCursor, kSensitivity));

    expectMatricesNear(controller.viewMatrix(), view_before_release);

    controller.beginOrbitDrag(kReleasedCursor);

    EXPECT_FALSE(controller.updateOrbitDrag(kReleasedCursor, kSensitivity));

    expectMatricesNear(controller.viewMatrix(), view_before_release);
}

TEST(SceneViewCameraControllerTests, RejectsInvalidDragSensitivity) {
    constexpr float kNegativeSensitivity = -0.01F;
    constexpr float kZeroSensitivity = 0.0F;

    constexpr Platform::CursorPosition kCursor{
        .x = 100.0,
        .y = 200.0,
    };

    constexpr auto kExpectedMessage =
        "Scene View orbit sensitivity must be finite and greater than zero";

    const float infinity = std::numeric_limits<float>::infinity();
    const float nan = std::numeric_limits<float>::quiet_NaN();

    Editor::SceneViewCameraController controller{};
    controller.beginOrbitDrag(kCursor);

    ASSERT_DEATH(static_cast<void>(
                     controller.updateOrbitDrag(kCursor, kZeroSensitivity)),
                 kExpectedMessage);

    ASSERT_DEATH(static_cast<void>(
                     controller.updateOrbitDrag(kCursor, kNegativeSensitivity)),
                 kExpectedMessage);

    ASSERT_DEATH(
        static_cast<void>(controller.updateOrbitDrag(kCursor, infinity)),
        kExpectedMessage);

    ASSERT_DEATH(static_cast<void>(controller.updateOrbitDrag(kCursor, nan)),
                 kExpectedMessage);
}

TEST(SceneViewCameraControllerTests, RejectsNonFiniteCursorUpdates) {
    constexpr float kSensitivity = 0.01F;
    constexpr double kValidCoordinate = 0.0;

    constexpr auto kExpectedMessage =
        "Scene View orbit cursor position must be finite";

    const double infinity = std::numeric_limits<double>::infinity();
    const double nan = std::numeric_limits<double>::quiet_NaN();

    Editor::SceneViewCameraController controller{};
    controller.beginOrbitDrag({});

    ASSERT_DEATH(static_cast<void>(controller.updateOrbitDrag(
                     {.x = infinity, .y = kValidCoordinate}, kSensitivity)),
                 kExpectedMessage);

    ASSERT_DEATH(static_cast<void>(controller.updateOrbitDrag(
                     {.x = kValidCoordinate, .y = infinity}, kSensitivity)),
                 kExpectedMessage);

    ASSERT_DEATH(static_cast<void>(controller.updateOrbitDrag(
                     {.x = nan, .y = kValidCoordinate}, kSensitivity)),
                 kExpectedMessage);

    ASSERT_DEATH(static_cast<void>(controller.updateOrbitDrag(
                     {.x = kValidCoordinate, .y = nan}, kSensitivity)),
                 kExpectedMessage);
}

TEST(SceneViewCameraControllerTests, RejectsOverflowingCursorMovement) {
    constexpr float kSensitivity = 0.01F;
    constexpr double kValidCoordinate = 0.0;

    constexpr auto kExpectedMessage =
        "Scene View orbit cursor movement produces invalid rotation angles";

    const double maximum = std::numeric_limits<double>::max();

    Editor::SceneViewCameraController controller{};

    controller.beginOrbitDrag({
        .x = -maximum,
        .y = kValidCoordinate,
    });

    ASSERT_DEATH(static_cast<void>(controller.updateOrbitDrag(
                     {.x = maximum, .y = kValidCoordinate}, kSensitivity)),
                 kExpectedMessage);
}

TEST(SceneViewCameraControllerTests, DefaultCameraHasExpectedWorldPosition) {
    const Editor::SceneViewCameraController controller{};

    const Math::Vector3f expected{
        .x = 0.0F,
        .y = 0.0F,
        .z = kDefaultDistance,
    };

    expectVectorsNear(controller.cameraWorldPosition(), expected);
}

TEST(SceneViewCameraControllerTests, PositiveYawUpdatesCameraWorldPosition) {
    Editor::SceneViewCameraController controller{};

    controller.orbit(kRightAngle, 0.0F);

    const Math::Vector3f expected{
        .x = kDefaultDistance,
        .y = 0.0F,
        .z = 0.0F,
    };

    expectVectorsNear(controller.cameraWorldPosition(), expected);
}

TEST(SceneViewCameraControllerTests, PositivePitchUpdatesCameraWorldPosition) {
    Editor::SceneViewCameraController controller{};

    controller.orbit(0.0F, kRightAngle);

    const Math::Vector3f expected{
        .x = 0.0F,
        .y = -kDefaultDistance,
        .z = 0.0F,
    };

    expectVectorsNear(controller.cameraWorldPosition(), expected);
}

TEST(SceneViewCameraControllerTests,
     CombinedOrbitProducesExpectedWorldPosition) {
    constexpr float kYawRadians = 0.4F;
    constexpr float kPitchRadians = -0.3F;

    Editor::SceneViewCameraController controller{};

    controller.orbit(kYawRadians, kPitchRadians);

    const Math::Vector3f expected{
        .x = kDefaultDistance * std::sin(kYawRadians) * std::cos(kPitchRadians),
        .y = -kDefaultDistance * std::sin(kPitchRadians),
        .z = kDefaultDistance * std::cos(kYawRadians) * std::cos(kPitchRadians),
    };

    expectVectorsNear(controller.cameraWorldPosition(), expected);
}

TEST(SceneViewCameraControllerTests, ViewMatrixMapsCameraPositionToOrigin) {
    constexpr float kYawRadians = 0.4F;
    constexpr float kPitchRadians = -0.3F;

    const Math::Vector3f kCameraSpaceOrigin{
        .x = 0.0F,
        .y = 0.0F,
        .z = 0.0F,
    };

    Editor::SceneViewCameraController controller{};

    controller.orbit(kYawRadians, kPitchRadians);

    const Math::Vector3f camera_position = controller.cameraWorldPosition();

    const Math::Vector3f transformed_position =
        Math::transformPoint(controller.viewMatrix(), camera_position);

    expectVectorsNear(transformed_position, kCameraSpaceOrigin);
}

TEST(SceneViewCameraControllerTests, PanTranslatesCameraAndViewMatrix) {
    constexpr Math::Vector3f kDisplacement{
        .x = 1.0F,
        .y = -2.0F,
        .z = 0.5F,
    };

    Editor::SceneViewCameraController controller{};

    controller.pan(kDisplacement);

    const Math::Vector3f expected_position{
        .x = kDisplacement.x,
        .y = kDisplacement.y,
        .z = kDefaultDistance + kDisplacement.z,
    };

    const Math::Matrix4f expected_view =
        Math::translate(-kDisplacement.x, -kDisplacement.y,
                        -(kDefaultDistance + kDisplacement.z));

    expectVectorsNear(controller.cameraWorldPosition(), expected_position);
    expectMatricesNear(controller.viewMatrix(), expected_view);
}

TEST(SceneViewCameraControllerTests, PanPreservesCameraOrientation) {
    constexpr Math::Vector3f kDisplacement{
        .x = 1.5F,
        .y = -0.75F,
        .z = 2.0F,
    };

    Editor::SceneViewCameraController controller{};
    controller.orbit(kRightAngle, 0.0F);

    const Math::Matrix4f original_view = controller.viewMatrix();

    controller.pan(kDisplacement);

    const Math::Vector3f expected_position{
        .x = kDefaultDistance + kDisplacement.x,
        .y = kDisplacement.y,
        .z = kDisplacement.z,
    };

    const Math::Matrix4f expected_view = Math::multiply(
        original_view,
        Math::translate(-kDisplacement.x, -kDisplacement.y, -kDisplacement.z));

    expectVectorsNear(controller.cameraWorldPosition(), expected_position);
    expectMatricesNear(controller.viewMatrix(), expected_view);
}

TEST(SceneViewCameraControllerTests, RepeatedPanAccumulatesDisplacements) {
    constexpr Math::Vector3f kFirstDisplacement{
        .x = 0.5F,
        .y = -1.0F,
        .z = 2.0F,
    };

    constexpr Math::Vector3f kSecondDisplacement{
        .x = -1.5F,
        .y = 3.0F,
        .z = -0.5F,
    };

    constexpr Math::Vector3f kZeroDisplacement{};

    Editor::SceneViewCameraController controller{};

    controller.pan(kFirstDisplacement);
    controller.pan(kSecondDisplacement);

    const Math::Vector3f expected_position{
        .x = -1.0F,
        .y = 2.0F,
        .z = kDefaultDistance + 1.5F,
    };

    expectVectorsNear(controller.cameraWorldPosition(), expected_position);

    const Math::Matrix4f view_before_zero_pan = controller.viewMatrix();

    controller.pan(kZeroDisplacement);

    expectMatricesNear(controller.viewMatrix(), view_before_zero_pan);
}

TEST(SceneViewCameraControllerTests, RejectsNonFinitePanDisplacement) {
    constexpr auto kExpectedMessage =
        "Scene View pan displacement must be finite";

    const float infinity = std::numeric_limits<float>::infinity();
    const float nan = std::numeric_limits<float>::quiet_NaN();

    Editor::SceneViewCameraController controller{};

    ASSERT_DEATH(controller.pan({.x = infinity}), kExpectedMessage);
    ASSERT_DEATH(controller.pan({.y = infinity}), kExpectedMessage);
    ASSERT_DEATH(controller.pan({.z = infinity}), kExpectedMessage);

    ASSERT_DEATH(controller.pan({.x = nan}), kExpectedMessage);
    ASSERT_DEATH(controller.pan({.y = nan}), kExpectedMessage);
    ASSERT_DEATH(controller.pan({.z = nan}), kExpectedMessage);
}

TEST(SceneViewCameraControllerTests, RejectsOverflowingPanTarget) {
    constexpr auto kExpectedMessage =
        "Scene View pan produces invalid orbit target";

    constexpr float kMaximumCoordinate = std::numeric_limits<float>::max();

    constexpr Math::Vector3f kLargeDisplacement{
        .x = kMaximumCoordinate,
    };

    Editor::SceneViewCameraController controller{};

    controller.pan(kLargeDisplacement);

    ASSERT_DEATH(controller.pan(kLargeDisplacement), kExpectedMessage);
}

TEST(SceneViewCameraControllerTests, CameraPlanePanUsesWorldAxesAtIdentity) {
    constexpr Math::Vector2f kDisplacement{
        .x = 2.0F,
        .y = -1.0F,
    };

    Editor::SceneViewCameraController controller{};

    controller.panCameraPlane(kDisplacement);

    const Math::Vector3f expected_position{
        .x = kDisplacement.x,
        .y = kDisplacement.y,
        .z = kDefaultDistance,
    };

    const Math::Matrix4f expected_view =
        Math::translate(-kDisplacement.x, -kDisplacement.y, -kDefaultDistance);

    expectVectorsNear(controller.cameraWorldPosition(), expected_position);
    expectMatricesNear(controller.viewMatrix(), expected_view);
}

TEST(SceneViewCameraControllerTests, CameraPlanePanUsesRotatedRightAxis) {
    constexpr float kRightMovement = 2.0F;

    constexpr Math::Vector2f kDisplacement{
        .x = kRightMovement,
    };

    Editor::SceneViewCameraController controller{};

    controller.orbit(kRightAngle, 0.0F);

    const Math::Matrix4f original_view = controller.viewMatrix();

    controller.panCameraPlane(kDisplacement);

    const Math::Vector3f expected_position{
        .x = kDefaultDistance,
        .y = 0.0F,
        .z = -kRightMovement,
    };

    const Math::Matrix4f expected_view = Math::multiply(
        original_view, Math::translate(0.0F, 0.0F, kRightMovement));

    expectVectorsNear(controller.cameraWorldPosition(), expected_position);
    expectMatricesNear(controller.viewMatrix(), expected_view);
}

TEST(SceneViewCameraControllerTests, CameraPlanePanUsesRotatedUpAxis) {
    constexpr float kUpMovement = 2.0F;

    constexpr Math::Vector2f kDisplacement{
        .y = kUpMovement,
    };

    Editor::SceneViewCameraController controller{};

    controller.orbit(0.0F, kRightAngle);

    const Math::Matrix4f original_view = controller.viewMatrix();

    controller.panCameraPlane(kDisplacement);

    const Math::Vector3f expected_position{
        .x = 0.0F,
        .y = -kDefaultDistance,
        .z = kUpMovement,
    };

    const Math::Matrix4f expected_view = Math::multiply(
        original_view, Math::translate(0.0F, 0.0F, -kUpMovement));

    expectVectorsNear(controller.cameraWorldPosition(), expected_position);
    expectMatricesNear(controller.viewMatrix(), expected_view);
}

TEST(SceneViewCameraControllerTests, RejectsNonFiniteCameraPlanePan) {
    constexpr auto kExpectedMessage =
        "Scene View camera-relative pan components must be finite";

    const float infinity = std::numeric_limits<float>::infinity();
    const float nan = std::numeric_limits<float>::quiet_NaN();

    Editor::SceneViewCameraController controller{};

    ASSERT_DEATH(controller.panCameraPlane({.x = infinity}), kExpectedMessage);

    ASSERT_DEATH(controller.panCameraPlane({.y = infinity}), kExpectedMessage);

    ASSERT_DEATH(controller.panCameraPlane({.x = nan}), kExpectedMessage);

    ASSERT_DEATH(controller.panCameraPlane({.y = nan}), kExpectedMessage);
}

TEST(SceneViewCameraControllerTests, PanDragLifecyclePreservesCameraTransform) {
    constexpr float kYawRadians = 0.3F;
    constexpr float kPitchRadians = -0.2F;

    constexpr Platform::CursorPosition kInitialCursor{
        .x = 100.0,
        .y = 200.0,
    };

    constexpr Platform::CursorPosition kRestartCursor{
        .x = 150.0,
        .y = 250.0,
    };

    Editor::SceneViewCameraController controller{};
    controller.orbit(kYawRadians, kPitchRadians);

    const Math::Matrix4f original_view = controller.viewMatrix();
    const Math::Vector3f original_position = controller.cameraWorldPosition();

    controller.beginPanDrag(kInitialCursor);

    expectMatricesNear(controller.viewMatrix(), original_view);
    expectVectorsNear(controller.cameraWorldPosition(), original_position);

    controller.beginPanDrag(kRestartCursor);

    expectMatricesNear(controller.viewMatrix(), original_view);
    expectVectorsNear(controller.cameraWorldPosition(), original_position);

    controller.endPanDrag();
    controller.endPanDrag();

    expectMatricesNear(controller.viewMatrix(), original_view);
    expectVectorsNear(controller.cameraWorldPosition(), original_position);
}

TEST(SceneViewCameraControllerTests, PanModePreventsOrbitUpdates) {
    constexpr float kSensitivity = 0.01F;

    constexpr Platform::CursorPosition kInitialCursor{
        .x = 100.0,
        .y = 200.0,
    };

    constexpr Platform::CursorPosition kPanCursor{
        .x = 150.0,
        .y = 250.0,
    };

    constexpr Platform::CursorPosition kMovedCursor{
        .x = 180.0,
        .y = 250.0,
    };

    Editor::SceneViewCameraController controller{};

    controller.beginOrbitDrag(kInitialCursor);
    controller.beginPanDrag(kPanCursor);

    const Math::Matrix4f original_view = controller.viewMatrix();

    EXPECT_FALSE(controller.updateOrbitDrag(kMovedCursor, kSensitivity));

    expectMatricesNear(controller.viewMatrix(), original_view);

    // Ending Orbit must not produce an Orbit update while Pan is active.
    controller.endOrbitDrag();

    EXPECT_FALSE(controller.updateOrbitDrag(kMovedCursor, kSensitivity));

    expectMatricesNear(controller.viewMatrix(), original_view);
}

TEST(SceneViewCameraControllerTests, EndingPanDoesNotInterruptOrbit) {
    constexpr float kSensitivity = 0.01F;
    constexpr float kExpectedYawRadians = -0.2F;

    constexpr Platform::CursorPosition kInitialCursor{
        .x = 100.0,
        .y = 200.0,
    };

    constexpr Platform::CursorPosition kMovedCursor{
        .x = 120.0,
        .y = 200.0,
    };

    Editor::SceneViewCameraController controller{};

    controller.beginOrbitDrag(kInitialCursor);
    controller.endPanDrag();

    ASSERT_TRUE(controller.updateOrbitDrag(kMovedCursor, kSensitivity));

    const Math::Matrix4f expected =
        Math::multiply(Math::translate(0.0F, 0.0F, -kDefaultDistance),
                       Math::rotateY(-kExpectedYawRadians));

    expectMatricesNear(controller.viewMatrix(), expected);
}

TEST(SceneViewCameraControllerTests, RejectsNonFinitePanDragCursor) {
    constexpr auto kExpectedMessage =
        "Scene View pan cursor position must be finite";

    const double infinity = std::numeric_limits<double>::infinity();
    const double nan = std::numeric_limits<double>::quiet_NaN();

    const Platform::CursorPosition infinite_x{.x = infinity};
    const Platform::CursorPosition infinite_y{.y = infinity};
    const Platform::CursorPosition nan_x{.x = nan};
    const Platform::CursorPosition nan_y{.y = nan};

    Editor::SceneViewCameraController controller{};

    ASSERT_DEATH(controller.beginPanDrag(infinite_x), kExpectedMessage);
    ASSERT_DEATH(controller.beginPanDrag(infinite_y), kExpectedMessage);
    ASSERT_DEATH(controller.beginPanDrag(nan_x), kExpectedMessage);
    ASSERT_DEATH(controller.beginPanDrag(nan_y), kExpectedMessage);
}

TEST(SceneViewCameraControllerTests, InactivePanIgnoresCursorMovement) {
    constexpr float kSensitivity = 0.01F;

    constexpr Platform::CursorPosition kCursor{
        .x = 120.0,
        .y = 180.0,
    };

    Editor::SceneViewCameraController controller{};

    const Math::Matrix4f original_view = controller.viewMatrix();

    EXPECT_FALSE(controller.updatePanDrag(kCursor, kSensitivity));

    expectMatricesNear(controller.viewMatrix(), original_view);
}

TEST(SceneViewCameraControllerTests, StationaryPanCursorDoesNotMoveCamera) {
    constexpr float kSensitivity = 0.01F;

    constexpr Platform::CursorPosition kCursor{
        .x = 100.0,
        .y = 200.0,
    };

    Editor::SceneViewCameraController controller{};
    controller.beginPanDrag(kCursor);

    const Math::Matrix4f original_view = controller.viewMatrix();

    EXPECT_FALSE(controller.updatePanDrag(kCursor, kSensitivity));

    expectMatricesNear(controller.viewMatrix(), original_view);
}

TEST(SceneViewCameraControllerTests,
     HorizontalAndVerticalPanWorkIndependently) {
    constexpr float kSensitivity = 0.01F;
    constexpr float kExpectedHorizontalPan = -0.2F;
    constexpr float kExpectedVerticalPan = 0.3F;

    constexpr Platform::CursorPosition kInitialCursor{
        .x = 100.0,
        .y = 200.0,
    };

    constexpr Platform::CursorPosition kHorizontalCursor{
        .x = 120.0,
        .y = 200.0,
    };

    constexpr Platform::CursorPosition kVerticalCursor{
        .x = 120.0,
        .y = 230.0,
    };

    Editor::SceneViewCameraController controller{};
    controller.beginPanDrag(kInitialCursor);

    ASSERT_TRUE(controller.updatePanDrag(kHorizontalCursor, kSensitivity));

    expectVectorsNear(
        controller.cameraWorldPosition(),
        {.x = kExpectedHorizontalPan, .y = 0.0F, .z = kDefaultDistance});

    ASSERT_TRUE(controller.updatePanDrag(kVerticalCursor, kSensitivity));

    expectVectorsNear(controller.cameraWorldPosition(),
                      {
                          .x = kExpectedHorizontalPan,
                          .y = kExpectedVerticalPan,
                          .z = kDefaultDistance,
                      });

    const Math::Matrix4f expected_view = Math::translate(
        -kExpectedHorizontalPan, -kExpectedVerticalPan, -kDefaultDistance);

    expectMatricesNear(controller.viewMatrix(), expected_view);
}

TEST(SceneViewCameraControllerTests, RepeatedPanUsesPreviousCursor) {
    constexpr float kSensitivity = 0.01F;
    constexpr float kExpectedPan = -0.3F;

    constexpr Platform::CursorPosition kInitialCursor{
        .x = 100.0,
        .y = 200.0,
    };

    constexpr Platform::CursorPosition kFirstCursor{
        .x = 110.0,
        .y = 200.0,
    };

    constexpr Platform::CursorPosition kSecondCursor{
        .x = 130.0,
        .y = 200.0,
    };

    Editor::SceneViewCameraController controller{};
    controller.beginPanDrag(kInitialCursor);

    ASSERT_TRUE(controller.updatePanDrag(kFirstCursor, kSensitivity));
    ASSERT_TRUE(controller.updatePanDrag(kSecondCursor, kSensitivity));

    expectVectorsNear(controller.cameraWorldPosition(),
                      {.x = kExpectedPan, .y = 0.0F, .z = kDefaultDistance});
}

TEST(SceneViewCameraControllerTests, EndingOrbitDoesNotInterruptActivePan) {
    constexpr float kSensitivity = 0.01F;
    constexpr float kExpectedPan = -0.2F;

    constexpr Platform::CursorPosition kInitialCursor{
        .x = 100.0,
        .y = 200.0,
    };

    constexpr Platform::CursorPosition kMovedCursor{
        .x = 120.0,
        .y = 200.0,
    };

    Editor::SceneViewCameraController controller{};

    controller.beginPanDrag(kInitialCursor);
    controller.endOrbitDrag();

    ASSERT_TRUE(controller.updatePanDrag(kMovedCursor, kSensitivity));

    expectVectorsNear(controller.cameraWorldPosition(),
                      {.x = kExpectedPan, .y = 0.0F, .z = kDefaultDistance});
}

TEST(SceneViewCameraControllerTests, RejectsInvalidPanDragSensitivity) {
    constexpr Platform::CursorPosition kCursor{
        .x = 100.0,
        .y = 200.0,
    };

    constexpr auto kExpectedMessage =
        "Scene View pan sensitivity must be finite and greater than zero";

    const float infinity = std::numeric_limits<float>::infinity();
    const float nan = std::numeric_limits<float>::quiet_NaN();

    Editor::SceneViewCameraController controller{};
    controller.beginPanDrag(kCursor);

    ASSERT_DEATH(static_cast<void>(controller.updatePanDrag(kCursor, 0.0F)),
                 kExpectedMessage);

    ASSERT_DEATH(static_cast<void>(controller.updatePanDrag(kCursor, -1.0F)),
                 kExpectedMessage);

    ASSERT_DEATH(static_cast<void>(controller.updatePanDrag(kCursor, infinity)),
                 kExpectedMessage);

    ASSERT_DEATH(static_cast<void>(controller.updatePanDrag(kCursor, nan)),
                 kExpectedMessage);
}

TEST(SceneViewCameraControllerTests, RejectsNonFinitePanCursorUpdates) {
    constexpr float kSensitivity = 0.01F;

    constexpr auto kExpectedMessage =
        "Scene View pan cursor position must be finite";

    const double infinity = std::numeric_limits<double>::infinity();
    const double nan = std::numeric_limits<double>::quiet_NaN();

    Editor::SceneViewCameraController controller{};
    controller.beginPanDrag({});

    ASSERT_DEATH(static_cast<void>(
                     controller.updatePanDrag({.x = infinity}, kSensitivity)),
                 kExpectedMessage);

    ASSERT_DEATH(static_cast<void>(
                     controller.updatePanDrag({.y = infinity}, kSensitivity)),
                 kExpectedMessage);

    ASSERT_DEATH(
        static_cast<void>(controller.updatePanDrag({.x = nan}, kSensitivity)),
        kExpectedMessage);

    ASSERT_DEATH(
        static_cast<void>(controller.updatePanDrag({.y = nan}, kSensitivity)),
        kExpectedMessage);
}

TEST(SceneViewCameraControllerTests, RejectsOverflowingPanCursorMovement) {
    constexpr float kSensitivity = 0.01F;
    constexpr double kExtremeCoordinate = std::numeric_limits<double>::max();

    constexpr auto kExpectedMessage =
        "Scene View pan cursor movement produces invalid displacement";

    Editor::SceneViewCameraController controller{};

    controller.beginPanDrag({.x = -kExtremeCoordinate, .y = 0.0});

    ASSERT_DEATH(static_cast<void>(controller.updatePanDrag(
                     {.x = kExtremeCoordinate, .y = 0.0}, kSensitivity)),
                 kExpectedMessage);
}
