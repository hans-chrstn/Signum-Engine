#include "engine/renderer/camera/camera_relative_position.hpp"
#include <array>
#include <cmath>
#include <cstddef>
#include <gtest/gtest.h>
#include <limits>
#include <optional>

namespace Renderer = SNE::Engine::Renderer;
namespace Math = SNE::Engine::Core::Math;

namespace {
    constexpr double kEarthOrbitDistanceMeters{149'597'870'700.0};

    constexpr double kFloatMaximum =
        static_cast<double>(std::numeric_limits<float>::max());
} // namespace

TEST(CameraRelativePositionTests, ComputesSignedRelativeCoordinates) {
    const Renderer::CameraRelativePositionInput input{
        .world_position = {.x = 10.0, .y = -20.0, .z = 30.0},
        .camera_position = {.x = 4.0, .y = -5.0, .z = 38.0},
    };

    const std::optional<Math::Vector3f> result =
        Renderer::tryMakeCameraRelativePosition(input);

    if (!result.has_value()) {
        ADD_FAILURE() << "Expected a valid camera-relative position";
        return;
    }

    EXPECT_FLOAT_EQ(result->x, 6.0F);
    EXPECT_FLOAT_EQ(result->y, -15.0F);
    EXPECT_FLOAT_EQ(result->z, -8.0F);
}

TEST(CameraRelativePositionTests, PreservesOffsetsAtAstronomicalDistances) {
    const Renderer::CameraRelativePositionInput input{
        .world_position =
            {
                .x = kEarthOrbitDistanceMeters + 12.5,
                .y = -kEarthOrbitDistanceMeters + 0.25,
                .z = kEarthOrbitDistanceMeters - 100.0,
            },
        .camera_position =
            {
                .x = kEarthOrbitDistanceMeters,
                .y = -kEarthOrbitDistanceMeters,
                .z = kEarthOrbitDistanceMeters,
            },
    };

    const std::optional<Math::Vector3f> result =
        Renderer::tryMakeCameraRelativePosition(input);

    if (!result.has_value()) {
        ADD_FAILURE() << "Expected a valid camera-relative position";
        return;
    }

    EXPECT_FLOAT_EQ(result->x, 12.5F);
    EXPECT_FLOAT_EQ(result->y, 0.25F);
    EXPECT_FLOAT_EQ(result->z, -100.0F);
}

TEST(CameraRelativePositionTests, ReturnsZeroForCoincidentPositions) {
    const Math::Vector3d position{
        .x = kEarthOrbitDistanceMeters,
        .y = -kEarthOrbitDistanceMeters,
        .z = 500.0,
    };

    const Renderer::CameraRelativePositionInput input{
        .world_position = position,
        .camera_position = position,
    };

    const std::optional<Math::Vector3f> result =
        Renderer::tryMakeCameraRelativePosition(input);

    if (!result.has_value()) {
        ADD_FAILURE() << "Expected a valid camera-relative position";
        return;
    }
    EXPECT_FLOAT_EQ(result->x, 0.0F);
    EXPECT_FLOAT_EQ(result->y, 0.0F);
    EXPECT_FLOAT_EQ(result->z, 0.0F);
}

TEST(CameraRelativePositionTests, AcceptsFiniteFloatBoundaries) {
    const Renderer::CameraRelativePositionInput input{
        .world_position =
            {
                .x = kFloatMaximum,
                .y = -kFloatMaximum,
                .z = 0.0,
            },
        .camera_position = {},
    };

    const std::optional<Math::Vector3f> result =
        Renderer::tryMakeCameraRelativePosition(input);

    if (!result.has_value()) {
        ADD_FAILURE() << "Expected a valid camera-relative position";
        return;
    }
    EXPECT_FLOAT_EQ(result->x, std::numeric_limits<float>::max());
    EXPECT_FLOAT_EQ(result->y, -std::numeric_limits<float>::max());
    EXPECT_FLOAT_EQ(result->z, 0.0F);
}

TEST(CameraRelativePositionTests, RejectsCoordinatesAboveFloatMaximum) {
    const double outside_range =
        std::nextafter(kFloatMaximum, std::numeric_limits<double>::infinity());

    const Renderer::CameraRelativePositionInput input{
        .world_position = {.x = outside_range},
        .camera_position = {},
    };

    EXPECT_EQ(Renderer::tryMakeCameraRelativePosition(input), std::nullopt);
}

TEST(CameraRelativePositionTests, RejectsCoordinatesBelowFloatMinimum) {
    const double outside_range = std::nextafter(
        -kFloatMaximum, -std::numeric_limits<double>::infinity());

    const Renderer::CameraRelativePositionInput input{
        .world_position = {.y = outside_range},
        .camera_position = {},
    };

    EXPECT_EQ(Renderer::tryMakeCameraRelativePosition(input), std::nullopt);
}

TEST(CameraRelativePositionTests, RejectsNonFiniteWorldCoordinates) {
    const std::array invalid_values{
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity(),
    };

    for (const double invalid_value : invalid_values) {
        const std::array inputs{
            Renderer::CameraRelativePositionInput{
                .world_position = {.x = invalid_value},
                .camera_position = {},
            },
            Renderer::CameraRelativePositionInput{
                .world_position = {.y = invalid_value},
                .camera_position = {},
            },
            Renderer::CameraRelativePositionInput{
                .world_position = {.z = invalid_value},
                .camera_position = {},
            },
        };

        for (std::size_t index{}; index < inputs.size(); ++index) {
            SCOPED_TRACE(index);

            EXPECT_EQ(Renderer::tryMakeCameraRelativePosition(inputs[index]),
                      std::nullopt);
        }
    }
}

TEST(CameraRelativePositionTests, RejectsNonFiniteCameraCoordinates) {
    const std::array invalid_values{
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity(),
    };

    for (const double invalid_value : invalid_values) {
        const std::array inputs{
            Renderer::CameraRelativePositionInput{
                .world_position = {},
                .camera_position = {.x = invalid_value},
            },
            Renderer::CameraRelativePositionInput{
                .world_position = {},
                .camera_position = {.y = invalid_value},
            },
            Renderer::CameraRelativePositionInput{
                .world_position = {},
                .camera_position = {.z = invalid_value},
            },
        };

        for (std::size_t index{}; index < inputs.size(); ++index) {
            SCOPED_TRACE(index);

            EXPECT_EQ(Renderer::tryMakeCameraRelativePosition(inputs[index]),
                      std::nullopt);
        }
    }
}

TEST(CameraRelativePositionTests, RejectsDoubleSubtractionOverflow) {
    constexpr double kDoubleMaximum = std::numeric_limits<double>::max();

    const Renderer::CameraRelativePositionInput input{
        .world_position = {.x = kDoubleMaximum},
        .camera_position = {.x = -kDoubleMaximum},
    };

    EXPECT_EQ(Renderer::tryMakeCameraRelativePosition(input), std::nullopt);
}

TEST(CameraRelativePositionTests, AcceptsSmallFractionalOffsets) {
    const Renderer::CameraRelativePositionInput input{
        .world_position = {.x = 0.125, .y = -0.5, .z = 1.25},
        .camera_position = {},
    };

    const std::optional<Math::Vector3f> result =
        Renderer::tryMakeCameraRelativePosition(input);

    if (!result.has_value()) {
        ADD_FAILURE() << "Expected a valid camera-relative position";
        return;
    }
    EXPECT_FLOAT_EQ(result->x, 0.125F);
    EXPECT_FLOAT_EQ(result->y, -0.5F);
    EXPECT_FLOAT_EQ(result->z, 1.25F);
}
