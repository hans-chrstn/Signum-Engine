#include "engine/core/math/vector4d.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <limits>

namespace Math = SNE::Engine::Core::Math;

static_assert([] -> bool {
    constexpr Math::Vector4d first{
        .x = 1.0,
        .y = 2.0,
        .z = 3.0,
        .w = 4.0,
    };

    constexpr Math::Vector4d second{
        .x = 4.0,
        .y = 5.0,
        .z = 6.0,
        .w = 7.0,
    };

    constexpr Math::Vector4d expected{
        .x = 5.0,
        .y = 7.0,
        .z = 9.0,
        .w = 11.0,
    };

    constexpr auto result = Math::add(first, second);

    return result.x == expected.x && result.y == expected.y &&
           result.z == expected.z && result.w == expected.w;
}());

static_assert([] -> bool {
    constexpr Math::Vector4d first{
        .x = 5.0,
        .y = 7.0,
        .z = 9.0,
        .w = 11.0,
    };

    constexpr Math::Vector4d second{
        .x = 1.0,
        .y = 2.0,
        .z = 3.0,
        .w = 4.0,
    };

    constexpr Math::Vector4d expected{
        .x = 4.0,
        .y = 5.0,
        .z = 6.0,
        .w = 7.0,
    };

    constexpr auto result = Math::subtract(first, second);

    return result.x == expected.x && result.y == expected.y &&
           result.z == expected.z && result.w == expected.w;
}());

static_assert([] -> bool {
    constexpr Math::Vector4d first{
        .x = 1.0,
        .y = 2.0,
        .z = 3.0,
        .w = 4.0,
    };

    constexpr Math::Vector4d second{
        .x = 4.0,
        .y = 5.0,
        .z = 6.0,
        .w = 7.0,
    };

    constexpr double result = Math::dot(first, second);
    constexpr double expected = 60.0;

    return result == expected;
}());

static_assert([] -> bool {
    constexpr Math::Vector4d vector{
        .x = 1.0,
        .y = 2.0,
        .z = 2.0,
        .w = 4.0,
    };

    constexpr double result = Math::lengthSquared(vector);
    constexpr double expected = 25.0;

    return result == expected;
}());

static_assert([] -> bool {
    constexpr Math::Vector4d vector{
        .x = 1.0,
        .y = 2.0,
        .z = 3.0,
        .w = 4.0,
    };

    constexpr double multiplier = 2.0;

    constexpr Math::Vector4d expected{
        .x = 2.0,
        .y = 4.0,
        .z = 6.0,
        .w = 8.0,
    };

    constexpr Math::Vector4d result = Math::scale(vector, multiplier);

    return result.x == expected.x && result.y == expected.y &&
           result.z == expected.z && result.w == expected.w;
}());

TEST(Vector4dTests, AddsVectorsCorrectly) {
    const Math::Vector4d vector_a{
        .x = 1.0,
        .y = 2.0,
        .z = 3.0,
        .w = 4.0,
    };

    const Math::Vector4d vector_b{
        .x = 4.0,
        .y = 5.0,
        .z = 6.0,
        .w = 7.0,
    };

    const Math::Vector4d expected{
        .x = 5.0,
        .y = 7.0,
        .z = 9.0,
        .w = 11.0,
    };

    const auto result = Math::add(vector_a, vector_b);

    EXPECT_DOUBLE_EQ(result.x, expected.x);
    EXPECT_DOUBLE_EQ(result.y, expected.y);
    EXPECT_DOUBLE_EQ(result.z, expected.z);
    EXPECT_DOUBLE_EQ(result.w, expected.w);
}

TEST(Vector4dTests, SubtractsVectorsCorrectly) {
    const Math::Vector4d vector_a{
        .x = 5.0,
        .y = 7.0,
        .z = 9.0,
        .w = 11.0,
    };

    const Math::Vector4d vector_b{
        .x = 1.0,
        .y = 2.0,
        .z = 3.0,
        .w = 4.0,
    };

    const Math::Vector4d expected{
        .x = 4.0,
        .y = 5.0,
        .z = 6.0,
        .w = 7.0,
    };

    const auto result = Math::subtract(vector_a, vector_b);

    EXPECT_DOUBLE_EQ(result.x, expected.x);
    EXPECT_DOUBLE_EQ(result.y, expected.y);
    EXPECT_DOUBLE_EQ(result.z, expected.z);
    EXPECT_DOUBLE_EQ(result.w, expected.w);
}

TEST(Vector4dTests, ComputesDotProductCorrectly) {
    const Math::Vector4d vector_a{
        .x = 1.0,
        .y = 2.0,
        .z = 3.0,
        .w = 4.0,
    };

    const Math::Vector4d vector_b{
        .x = 4.0,
        .y = 5.0,
        .z = 6.0,
        .w = 7.0,
    };

    const double expected = 60.0;
    const auto result = Math::dot(vector_a, vector_b);

    EXPECT_DOUBLE_EQ(result, expected);
}

TEST(Vector4dTests, PerpendicularVectorsHaveZeroDotProduct) {
    const Math::Vector4d vector_a{
        .x = 1.0,
        .y = 0.0,
        .z = 0.0,
        .w = 0.0,
    };

    const Math::Vector4d vector_b{
        .x = 0.0,
        .y = 0.0,
        .z = 0.0,
        .w = 1.0,
    };

    const auto result = Math::dot(vector_a, vector_b);
    const double expected = 0.0;

    EXPECT_DOUBLE_EQ(result, expected);
}

TEST(Vector4dTests, OppositeVectorsHaveNegativeDotProduct) {
    const Math::Vector4d vector_a{
        .x = 0.0,
        .y = 0.0,
        .z = 0.0,
        .w = 1.0,
    };

    const Math::Vector4d vector_b{
        .x = 0.0,
        .y = 0.0,
        .z = 0.0,
        .w = -1.0,
    };

    const auto result = Math::dot(vector_a, vector_b);
    const double expected = -1.0;

    EXPECT_DOUBLE_EQ(result, expected);
}

TEST(Vector4dTests, ComputesSquaredLengthCorrectly) {
    const Math::Vector4d vector{
        .x = 1.0,
        .y = 2.0,
        .z = 2.0,
        .w = 4.0,
    };

    const auto result = Math::lengthSquared(vector);
    const double expected = 25.0;

    EXPECT_DOUBLE_EQ(result, expected);
}

TEST(Vector4dTests, ComputesLengthCorrectly) {
    const Math::Vector4d vector{
        .x = 1.0,
        .y = 2.0,
        .z = 2.0,
        .w = 4.0,
    };

    const auto result = Math::length(vector);
    const double expected = 5.0;

    EXPECT_DOUBLE_EQ(result, expected);
}

TEST(Vector4dTests, NormalizesVectorCorrectly) {
    const Math::Vector4d vector{
        .x = 0.0,
        .y = 3.0,
        .z = 0.0,
        .w = 4.0,
    };

    const auto result = Math::normalize(vector);

    const Math::Vector4d expected{
        .x = 0.0,
        .y = 0.6,
        .z = 0.0,
        .w = 0.8,
    };

    EXPECT_DOUBLE_EQ(result.x, expected.x);
    EXPECT_DOUBLE_EQ(result.y, expected.y);
    EXPECT_DOUBLE_EQ(result.z, expected.z);
    EXPECT_DOUBLE_EQ(result.w, expected.w);
}

TEST(Vector4dTests, NormalizingZeroVectorTriggersPrecondition) {
    const Math::Vector4d vector{
        .x = 0.0,
        .y = 0.0,
        .z = 0.0,
        .w = 0.0,
    };

    EXPECT_DEATH(static_cast<void>(Math::normalize(vector)),
                 "Cannot normalize a vector with an invalid magnitude");
}

TEST(Vector4dTests, ScalesVectorCorrectly) {
    const Math::Vector4d vector{
        .x = 1.0,
        .y = 2.0,
        .z = 3.0,
        .w = 4.0,
    };

    const double multiplier = 2.0;
    const auto result = Math::scale(vector, multiplier);

    const Math::Vector4d expected{
        .x = 2.0,
        .y = 4.0,
        .z = 6.0,
        .w = 8.0,
    };

    EXPECT_DOUBLE_EQ(result.x, expected.x);
    EXPECT_DOUBLE_EQ(result.y, expected.y);
    EXPECT_DOUBLE_EQ(result.z, expected.z);
    EXPECT_DOUBLE_EQ(result.w, expected.w);
}

TEST(Vector4dTests, DefaultInitializationProducesFourZeros) {
    constexpr double kExpectedZero = 0.0;

    const Math::Vector4d vector{};

    EXPECT_DOUBLE_EQ(vector.x, kExpectedZero);
    EXPECT_DOUBLE_EQ(vector.y, kExpectedZero);
    EXPECT_DOUBLE_EQ(vector.z, kExpectedZero);
    EXPECT_DOUBLE_EQ(vector.w, kExpectedZero);
}

TEST(Vector4dTests, NormalizesVectorWithOnlyWComponent) {
    const Math::Vector4d vector{
        .x = 0.0,
        .y = 0.0,
        .z = 0.0,
        .w = -7.0,
    };

    const Math::Vector4d expected{
        .x = 0.0,
        .y = 0.0,
        .z = 0.0,
        .w = -1.0,
    };

    const auto result = Math::normalize(vector);

    EXPECT_DOUBLE_EQ(result.x, expected.x);
    EXPECT_DOUBLE_EQ(result.y, expected.y);
    EXPECT_DOUBLE_EQ(result.z, expected.z);
    EXPECT_DOUBLE_EQ(result.w, expected.w);
}

TEST(Vector4dTests, ComputesFourDimensionalEuclideanLength) {
    const Math::Vector4d vector{
        .x = 3.0,
        .y = 4.0,
        .z = 0.0,
        .w = 12.0,
    };

    const double result = Math::length(vector);

    EXPECT_DOUBLE_EQ(result, 13.0);
}

TEST(Vector4dTests, ComputesLargeMagnitudeWithoutOverflow) {
    constexpr double kLargeComponent = 1.0e200;
    constexpr double kExpectedScaledMagnitude = 2.0;
    constexpr double kTolerance = 1.0e-12;

    const Math::Vector4d vector{
        .x = kLargeComponent,
        .y = kLargeComponent,
        .z = kLargeComponent,
        .w = kLargeComponent,
    };

    const double result = Math::length(vector);

    EXPECT_TRUE(std::isfinite(result));
    EXPECT_NEAR(result / kLargeComponent, kExpectedScaledMagnitude, kTolerance);
}

TEST(Vector4dTests, NormalizesVerySmallNonzeroVector) {
    constexpr double kSmallComponent = 1.0e-200;

    const Math::Vector4d vector{
        .x = 0.0,
        .y = 0.0,
        .z = 0.0,
        .w = kSmallComponent,
    };

    const Math::Vector4d expected{
        .x = 0.0,
        .y = 0.0,
        .z = 0.0,
        .w = 1.0,
    };

    const double magnitude = Math::length(vector);
    const Math::Vector4d result = Math::normalize(vector);

    EXPECT_DOUBLE_EQ(magnitude, kSmallComponent);
    EXPECT_DOUBLE_EQ(result.x, expected.x);
    EXPECT_DOUBLE_EQ(result.y, expected.y);
    EXPECT_DOUBLE_EQ(result.z, expected.z);
    EXPECT_DOUBLE_EQ(result.w, expected.w);
}

TEST(Vector4dTests, PreservesDoublePrecisionAtLargeCoordinates) {
    constexpr double kLargeCoordinate = 1.0e12;
    constexpr double kSmallOffset = 0.25;
    constexpr double kExpectedCoordinate = 1000000000000.25;

    const Math::Vector4d position{
        .x = 0.0,
        .y = 0.0,
        .z = 0.0,
        .w = kLargeCoordinate,
    };

    const Math::Vector4d offset{
        .x = 0.0,
        .y = 0.0,
        .z = 0.0,
        .w = kSmallOffset,
    };

    const Math::Vector4d result = Math::add(position, offset);

    EXPECT_DOUBLE_EQ(result.x, 0.0);
    EXPECT_DOUBLE_EQ(result.y, 0.0);
    EXPECT_DOUBLE_EQ(result.z, 0.0);
    EXPECT_DOUBLE_EQ(result.w, kExpectedCoordinate);
}

TEST(Vector4dTests, RejectsNonFiniteNormalization) {
    const Math::Vector4d infinite_vector{
        .x = 0.0,
        .y = 1.0,
        .z = 0.0,
        .w = std::numeric_limits<double>::infinity(),
    };

    const Math::Vector4d nan_vector{
        .x = 0.0,
        .y = std::numeric_limits<double>::quiet_NaN(),
        .z = 1.0,
        .w = 0.0,
    };

    EXPECT_DEATH(static_cast<void>(Math::normalize(infinite_vector)),
                 "Cannot normalize a vector with an invalid magnitude");

    EXPECT_DEATH(static_cast<void>(Math::normalize(nan_vector)),
                 "Cannot normalize a vector with an invalid magnitude");
}
