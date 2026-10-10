#include "engine/core/math/vector2d.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <limits>
#include <numbers>

namespace Math = SNE::Engine::Core::Math;

static_assert([] -> bool {
    constexpr Math::Vector2d first{.x = 1.0};
    constexpr Math::Vector2d second{.y = 1.0};

    constexpr auto result = Math::add(first, second);

    return result.x == 1.0 && result.y == 1.0;
}());

static_assert([] -> bool {
    constexpr Math::Vector2d first{.x = 1.0};
    constexpr Math::Vector2d second{.y = 1.0};

    constexpr auto result = Math::subtract(first, second);

    return result.x == 1.0 && result.y == -1.0;
}());

static_assert([] -> bool {
    constexpr Math::Vector2d vector{
        .x = 3.0,
        .y = 4.0,
    };

    constexpr double result = Math::lengthSquared(vector);
    constexpr double expected = 25.0;

    return result == expected;
}());

static_assert([] -> bool {
    constexpr Math::Vector2d vector{
        .x = 1.0,
        .y = 2.0,
    };

    constexpr double multiplier = 2.0;

    constexpr Math::Vector2d expected{
        .x = 2.0,
        .y = 4.0,
    };

    constexpr Math::Vector2d result = Math::scale(vector, multiplier);

    return result.x == expected.x && result.y == expected.y;
}());

TEST(Vector2dTests, AddsVectorsCorrectly) {
    const Math::Vector2d vector_a{
        .x = 1.0,
        .y = 2.0,
    };

    const Math::Vector2d vector_b{
        .x = 4.0,
        .y = 5.0,
    };

    const Math::Vector2d expected{
        .x = 5.0,
        .y = 7.0,
    };

    const auto result = Math::add(vector_a, vector_b);

    EXPECT_DOUBLE_EQ(result.x, expected.x);
    EXPECT_DOUBLE_EQ(result.y, expected.y);
}

TEST(Vector2dTests, SubtractsVectorsCorrectly) {
    const Math::Vector2d vector_a{
        .x = 5.0,
        .y = 7.0,
    };

    const Math::Vector2d vector_b{
        .x = 1.0,
        .y = 2.0,
    };

    const Math::Vector2d expected{
        .x = 4.0,
        .y = 5.0,
    };

    const auto result = Math::subtract(vector_a, vector_b);

    EXPECT_DOUBLE_EQ(result.x, expected.x);
    EXPECT_DOUBLE_EQ(result.y, expected.y);
}

TEST(Vector2dTests, ComputesDotProductCorrectly) {
    const Math::Vector2d vector_a{
        .x = 1.0,
        .y = 2.0,
    };

    const Math::Vector2d vector_b{
        .x = 4.0,
        .y = 5.0,
    };

    const double expected = 14.0;

    const auto result = Math::dot(vector_a, vector_b);
    EXPECT_DOUBLE_EQ(result, expected);
}

TEST(Vector2dTests, PerpendicularVectorsHaveZeroDotProduct) {
    const Math::Vector2d vector_a{
        .x = 1.0,
        .y = 0.0,
    };

    const Math::Vector2d vector_b{
        .x = 0.0,
        .y = 1.0,
    };

    const auto result = Math::dot(vector_a, vector_b);
    const double expected = 0.0;

    EXPECT_DOUBLE_EQ(result, expected);
}

TEST(Vector2dTests, OppositeVectorsHaveNegativeDotProduct) {
    const Math::Vector2d vector_a{
        .x = 1.0,
        .y = 0.0,
    };

    const Math::Vector2d vector_b{
        .x = -1.0,
        .y = 0.0,
    };

    const auto result = Math::dot(vector_a, vector_b);
    const double expected = -1.0;

    EXPECT_DOUBLE_EQ(result, expected);
}

TEST(Vector2dTests, ComputesSquaredLengthCorrectly) {
    const Math::Vector2d vector{
        .x = 3.0,
        .y = 4.0,
    };

    const auto result = Math::lengthSquared(vector);
    const double expected = 25.0;
    EXPECT_DOUBLE_EQ(result, expected);
}

TEST(Vector2dTests, ComputesLengthCorrectly) {
    const Math::Vector2d vector{
        .x = 3.0,
        .y = 4.0,
    };

    const auto result = Math::length(vector);
    const double expected = 5.0;
    EXPECT_DOUBLE_EQ(result, expected);
}

TEST(Vector2dTests, NormalizesVectorCorrectly) {
    const Math::Vector2d vector{
        .x = 3.0,
        .y = 4.0,
    };

    const auto result = Math::normalize(vector);
    const Math::Vector2d expected{
        .x = 0.6,
        .y = 0.8,
    };

    EXPECT_DOUBLE_EQ(result.x, expected.x);
    EXPECT_DOUBLE_EQ(result.y, expected.y);
}

TEST(Vector2dTests, NormalizingZeroVectorTriggersPrecondition) {
    const Math::Vector2d vector{
        .x = 0.0,
        .y = 0.0,
    };

    EXPECT_DEATH(static_cast<void>(Math::normalize(vector)),
                 "Cannot normalize a vector with an invalid magnitude");
}

TEST(Vector2dTests, ScalesVectorCorrectly) {
    const Math::Vector2d vector{
        .x = 1.0,
        .y = 2.0,
    };

    const double multiplier = 2.0;

    const auto result = Math::scale(vector, multiplier);
    const Math::Vector2d expected{
        .x = 2.0,
        .y = 4.0,
    };

    EXPECT_DOUBLE_EQ(result.x, expected.x);
    EXPECT_DOUBLE_EQ(result.y, expected.y);
}

TEST(Vector2dTests, ComputesLargeMagnitudeWithoutOverflow) {
    constexpr double kLargeComponent = 1.0e200;
    constexpr double kExpectedScaledMagnitude = std::numbers::sqrt2_v<double>;
    constexpr double kTolerance = 1.0e-12;

    const Math::Vector2d vector{
        .x = kLargeComponent,
        .y = kLargeComponent,
    };

    const double result = Math::length(vector);

    EXPECT_TRUE(std::isfinite(result));
    EXPECT_NEAR(result / kLargeComponent, kExpectedScaledMagnitude, kTolerance);
}

TEST(Vector2dTests, NormalizesVerySmallNonzeroVector) {
    constexpr double kSmallComponent = 1.0e-200;

    const Math::Vector2d vector{
        .x = kSmallComponent,
        .y = 0.0,
    };

    const Math::Vector2d expected{
        .x = 1.0,
        .y = 0.0,
    };

    const double magnitude = Math::length(vector);
    const Math::Vector2d result = Math::normalize(vector);

    EXPECT_DOUBLE_EQ(magnitude, kSmallComponent);
    EXPECT_DOUBLE_EQ(result.x, expected.x);
    EXPECT_DOUBLE_EQ(result.y, expected.y);
}

TEST(Vector2dTests, PreservesDoublePrecisionAtLargeCoordinates) {
    constexpr double kLargeCoordinate = 1.0e9;
    constexpr double kSmallOffset = 1.0;
    constexpr double kExpectedCoordinate = 1000000001.0;

    const Math::Vector2d position{
        .x = kLargeCoordinate,
        .y = 0.0,
    };

    const Math::Vector2d offset{
        .x = kSmallOffset,
        .y = 0.0,
    };

    const Math::Vector2d result = Math::add(position, offset);

    EXPECT_DOUBLE_EQ(result.x, kExpectedCoordinate);
    EXPECT_DOUBLE_EQ(result.y, 0.0);
}

TEST(Vector2dTests, RejectsNonFiniteNormalization) {
    const Math::Vector2d infinite_vector{
        .x = std::numeric_limits<double>::infinity(),
        .y = 1.0,
    };

    const Math::Vector2d nan_vector{
        .x = std::numeric_limits<double>::quiet_NaN(),
        .y = 1.0,
    };

    EXPECT_DEATH(static_cast<void>(Math::normalize(infinite_vector)),
                 "Cannot normalize a vector with an invalid magnitude");

    EXPECT_DEATH(static_cast<void>(Math::normalize(nan_vector)),
                 "Cannot normalize a vector with an invalid magnitude");
}
