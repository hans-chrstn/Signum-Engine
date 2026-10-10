#include "engine/core/math/vector3d.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <limits>
#include <numbers>

namespace Math = SNE::Engine::Core::Math;

static_assert([] -> bool {
    constexpr Math::Vector3d first{.x = 1.0};
    constexpr Math::Vector3d second{.y = 1.0};

    constexpr auto result = Math::add(first, second);

    return result.x == 1.0 && result.y == 1.0 && result.z == 0.0;
}());

static_assert([] -> bool {
    constexpr Math::Vector3d first{.x = 1.0};
    constexpr Math::Vector3d second{.y = 1.0};

    constexpr auto result = Math::subtract(first, second);

    return result.x == 1.0 && result.y == -1.0 && result.z == 0.0;
}());

static_assert([] -> bool {
    constexpr Math::Vector3d first{
        .x = 1.0,
        .y = 2.0,
        .z = 3.0,
    };

    constexpr Math::Vector3d second{
        .x = 4.0,
        .y = 5.0,
        .z = 6.0,
    };

    constexpr double result = Math::dot(first, second);
    constexpr double expected = 32.0;

    return result == expected;
}());

static_assert([] -> bool {
    constexpr Math::Vector3d first{
        .x = 1.0,
        .y = 0.0,
        .z = 0.0,
    };

    constexpr Math::Vector3d second{
        .x = 0.0,
        .y = 1.0,
        .z = 0.0,
    };

    constexpr Math::Vector3d result = Math::cross(first, second);

    return result.x == 0.0 && result.y == 0.0 && result.z == 1.0;
}());

static_assert([] -> bool {
    constexpr Math::Vector3d vector{
        .x = 3.0,
        .y = 4.0,
        .z = 0.0,
    };

    constexpr double result = Math::lengthSquared(vector);
    constexpr double expected = 25.0;

    return result == expected;
}());

static_assert([] -> bool {
    constexpr Math::Vector3d vector{
        .x = 1.0,
        .y = 2.0,
        .z = 3.0,
    };

    constexpr double multiplier = 2.0;

    constexpr Math::Vector3d expected{
        .x = 2.0,
        .y = 4.0,
        .z = 6.0,
    };

    constexpr Math::Vector3d result = Math::scale(vector, multiplier);

    return result.x == expected.x && result.y == expected.y &&
           result.z == expected.z;
}());

TEST(Vector3dTests, AddsVectorsCorrectly) {
    const Math::Vector3d vector_a{
        .x = 1.0,
        .y = 2.0,
        .z = 3.0,
    };

    const Math::Vector3d vector_b{
        .x = 4.0,
        .y = 5.0,
        .z = 6.0,
    };

    const Math::Vector3d expected{
        .x = 5.0,
        .y = 7.0,
        .z = 9.0,
    };

    const auto result = Math::add(vector_a, vector_b);

    EXPECT_DOUBLE_EQ(result.x, expected.x);
    EXPECT_DOUBLE_EQ(result.y, expected.y);
    EXPECT_DOUBLE_EQ(result.z, expected.z);
}

TEST(Vector3dTests, SubtractsVectorsCorrectly) {
    const Math::Vector3d vector_a{
        .x = 5.0,
        .y = 7.0,
        .z = 9.0,
    };

    const Math::Vector3d vector_b{
        .x = 1.0,
        .y = 2.0,
        .z = 3.0,
    };

    const Math::Vector3d expected{
        .x = 4.0,
        .y = 5.0,
        .z = 6.0,
    };

    const auto result = Math::subtract(vector_a, vector_b);

    EXPECT_DOUBLE_EQ(result.x, expected.x);
    EXPECT_DOUBLE_EQ(result.y, expected.y);
    EXPECT_DOUBLE_EQ(result.z, expected.z);
}

TEST(Vector3dTests, ComputesDotProductCorrectly) {
    const Math::Vector3d vector_a{
        .x = 1.0,
        .y = 2.0,
        .z = 3.0,
    };

    const Math::Vector3d vector_b{
        .x = 4.0,
        .y = 5.0,
        .z = 6.0,
    };

    const double expected = 32.0;

    const auto result = Math::dot(vector_a, vector_b);
    EXPECT_DOUBLE_EQ(result, expected);
}

TEST(Vector3dTests, PerpendicularVectorsHaveZeroDotProduct) {
    const Math::Vector3d vector_a{
        .x = 1.0,
        .y = 0.0,
        .z = 0.0,
    };

    const Math::Vector3d vector_b{
        .x = 0.0,
        .y = 1.0,
        .z = 0.0,
    };

    const auto result = Math::dot(vector_a, vector_b);
    const double expected = 0.0;

    EXPECT_DOUBLE_EQ(result, expected);
}

TEST(Vector3dTests, OppositeVectorsHaveNegativeDotProduct) {
    const Math::Vector3d vector_a{
        .x = 1.0,
        .y = 0.0,
        .z = 0.0,
    };

    const Math::Vector3d vector_b{
        .x = -1.0,
        .y = 0.0,
        .z = 0.0,
    };

    const auto result = Math::dot(vector_a, vector_b);
    const double expected = -1.0;

    EXPECT_DOUBLE_EQ(result, expected);
}

TEST(Vector3dTests, ComputesCrossProductCorrectly) {
    const Math::Vector3d vector_a{
        .x = 1.0,
        .y = 2.0,
        .z = 3.0,
    };

    const Math::Vector3d vector_b{
        .x = 4.0,
        .y = 5.0,
        .z = 6.0,
    };

    const auto result = Math::cross(vector_a, vector_b);
    const Math::Vector3d expected{
        .x = -3.0,
        .y = 6.0,
        .z = -3.0,
    };

    EXPECT_DOUBLE_EQ(result.x, expected.x);
    EXPECT_DOUBLE_EQ(result.y, expected.y);
    EXPECT_DOUBLE_EQ(result.z, expected.z);
}

TEST(Vector3dTests, ReversingOperandsNegatesCrossProduct) {
    const Math::Vector3d vector_a{
        .x = 1.0,
        .y = 2.0,
        .z = 3.0,
    };

    const Math::Vector3d vector_b{
        .x = 4.0,
        .y = 5.0,
        .z = 6.0,
    };

    const auto result = Math::cross(vector_b, vector_a);
    const Math::Vector3d expected{
        .x = 3.0,
        .y = -6.0,
        .z = 3.0,
    };

    EXPECT_DOUBLE_EQ(result.x, expected.x);
    EXPECT_DOUBLE_EQ(result.y, expected.y);
    EXPECT_DOUBLE_EQ(result.z, expected.z);
}

TEST(Vector3dTests, ComputesSquaredLengthCorrectly) {
    const Math::Vector3d vector{
        .x = 3.0,
        .y = 4.0,
        .z = 0.0,
    };

    const auto result = Math::lengthSquared(vector);
    const double expected = 25.0;

    EXPECT_DOUBLE_EQ(result, expected);
}

TEST(Vector3dTests, ComputesLengthCorrectly) {
    const Math::Vector3d vector{
        .x = 3.0,
        .y = 4.0,
        .z = 0.0,
    };

    const auto result = Math::length(vector);
    const double expected = 5.0;

    EXPECT_DOUBLE_EQ(result, expected);
}

TEST(Vector3dTests, NormalizesVectorCorrectly) {
    const Math::Vector3d vector{
        .x = 3.0,
        .y = 4.0,
        .z = 0.0,
    };

    const auto result = Math::normalize(vector);
    const Math::Vector3d expected{
        .x = 0.6,
        .y = 0.8,
        .z = 0.0,
    };

    EXPECT_DOUBLE_EQ(result.x, expected.x);
    EXPECT_DOUBLE_EQ(result.y, expected.y);
    EXPECT_DOUBLE_EQ(result.z, expected.z);
}

TEST(Vector3dTests, NormalizingZeroVectorTriggersPrecondition) {
    const Math::Vector3d vector{
        .x = 0.0,
        .y = 0.0,
        .z = 0.0,
    };

    EXPECT_DEATH(static_cast<void>(Math::normalize(vector)),
                 "Cannot normalize a vector with an invalid magnitude");
}

TEST(Vector3dTests, ScalesVectorCorrectly) {
    const Math::Vector3d vector{
        .x = 1.0,
        .y = 2.0,
        .z = 3.0,
    };

    const double multiplier = 2.0;

    const auto result = Math::scale(vector, multiplier);
    const Math::Vector3d expected{
        .x = 2.0,
        .y = 4.0,
        .z = 6.0,
    };

    EXPECT_DOUBLE_EQ(result.x, expected.x);
    EXPECT_DOUBLE_EQ(result.y, expected.y);
    EXPECT_DOUBLE_EQ(result.z, expected.z);
}

TEST(Vector3dTests, DefaultInitializationProducesThreeZeros) {
    const Math::Vector3d vector{};

    EXPECT_DOUBLE_EQ(vector.x, 0.0);
    EXPECT_DOUBLE_EQ(vector.y, 0.0);
    EXPECT_DOUBLE_EQ(vector.z, 0.0);
}

TEST(Vector3dTests, ComputesLargeMagnitudeWithoutOverflow) {
    constexpr double kLargeComponent = 1.0e200;
    constexpr double kExpectedScaledMagnitude = std::numbers::sqrt3_v<double>;
    constexpr double kTolerance = 1.0e-12;

    const Math::Vector3d vector{
        .x = kLargeComponent,
        .y = kLargeComponent,
        .z = kLargeComponent,
    };

    const double result = Math::length(vector);

    EXPECT_TRUE(std::isfinite(result));
    EXPECT_NEAR(result / kLargeComponent, kExpectedScaledMagnitude, kTolerance);
}

TEST(Vector3dTests, NormalizesVerySmallNonzeroVector) {
    constexpr double kSmallComponent = 1.0e-200;

    const Math::Vector3d vector{
        .x = kSmallComponent,
        .y = 0.0,
        .z = 0.0,
    };

    const Math::Vector3d expected{
        .x = 1.0,
        .y = 0.0,
        .z = 0.0,
    };

    const double magnitude = Math::length(vector);
    const Math::Vector3d result = Math::normalize(vector);

    EXPECT_DOUBLE_EQ(magnitude, kSmallComponent);
    EXPECT_DOUBLE_EQ(result.x, expected.x);
    EXPECT_DOUBLE_EQ(result.y, expected.y);
    EXPECT_DOUBLE_EQ(result.z, expected.z);
}

TEST(Vector3dTests, PreservesDoublePrecisionAtLargeCoordinates) {
    constexpr double kLargeCoordinate = 1.0e12;
    constexpr double kSmallOffset = 0.25;
    constexpr double kExpectedCoordinate = 1000000000000.25;

    const Math::Vector3d position{
        .x = kLargeCoordinate,
        .y = 0.0,
        .z = 0.0,
    };

    const Math::Vector3d offset{
        .x = kSmallOffset,
        .y = 0.0,
        .z = 0.0,
    };

    const Math::Vector3d result = Math::add(position, offset);

    EXPECT_DOUBLE_EQ(result.x, kExpectedCoordinate);
    EXPECT_DOUBLE_EQ(result.y, 0.0);
    EXPECT_DOUBLE_EQ(result.z, 0.0);
}

TEST(Vector3dTests, RejectsNonFiniteNormalization) {
    const Math::Vector3d infinite_vector{
        .x = 0.0,
        .y = 1.0,
        .z = std::numeric_limits<double>::infinity(),
    };

    const Math::Vector3d nan_vector{
        .x = std::numeric_limits<double>::quiet_NaN(),
        .y = 1.0,
        .z = 0.0,
    };

    EXPECT_DEATH(static_cast<void>(Math::normalize(infinite_vector)),
                 "Cannot normalize a vector with an invalid magnitude");

    EXPECT_DEATH(static_cast<void>(Math::normalize(nan_vector)),
                 "Cannot normalize a vector with an invalid magnitude");
}
