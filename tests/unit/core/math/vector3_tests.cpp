#include "engine/core/math/vector3.hpp"
#include <gtest/gtest.h>

namespace Math = SNE::Engine::Core::Math;

static_assert([] -> bool {
    constexpr Math::Vector3f first{.x = 1.0F};
    constexpr Math::Vector3f second{.y = 1.0F};

    constexpr auto result = Math::add(first, second);

    return result.x == 1.0F && result.y == 1.0F && result.z == 0.0F;
}());

static_assert([] -> bool {
    constexpr Math::Vector3f first{.x = 1.0F};
    constexpr Math::Vector3f second{.y = 1.0F};

    constexpr auto result = Math::subtract(first, second);

    return result.x == 1.0F && result.y == -1.0F && result.z == 0.0F;
}());

static_assert([] -> bool {
    constexpr Math::Vector3f first{
        .x = 1.0F,
        .y = 2.0F,
        .z = 3.0F,
    };

    constexpr Math::Vector3f second{
        .x = 4.0F,
        .y = 5.0F,
        .z = 6.0F,
    };

    constexpr float result = Math::dot(first, second);

    const float expected = 32.0F;

    return result == expected;
}());

static_assert([] -> bool {
    constexpr Math::Vector3f first{
        .x = 1.0F,
        .y = 0.0F,
        .z = 0.0F,
    };

    constexpr Math::Vector3f second{
        .x = 0.0F,
        .y = 1.0F,
        .z = 0.0F,
    };

    constexpr Math::Vector3f result = Math::cross(first, second);

    return result.x == 0.0F && result.y == 0.0F && result.z == 1.0F;
}());

static_assert([] -> bool {
    constexpr Math::Vector3f vector{
        .x = 3.0F,
        .y = 4.0F,
        .z = 0.0F,
    };

    constexpr float result = Math::lengthSquared(vector);
    constexpr float expected = 25.0F;

    return result == expected;
}());

static_assert([] -> bool {
    constexpr Math::Vector3f vector{
        .x = 1.0F,
        .y = 2.0F,
        .z = 3.0F,
    };

    constexpr float multiplier = 2.0F;

    constexpr Math::Vector3f expected{
        .x = 2.0F,
        .y = 4.0F,
        .z = 6.0F,
    };

    constexpr Math::Vector3f result = Math::scale(vector, multiplier);

    return result.x == expected.x && result.y == expected.y &&
           result.z == expected.z;
}());

TEST(Vector3Tests, AddsVectorsCorrectly) {
    const Math::Vector3f vector_a{
        .x = 1.0F,
        .y = 2.0F,
        .z = 3.0F,
    };

    const Math::Vector3f vector_b{
        .x = 4.0F,
        .y = 5.0F,
        .z = 6.0F,
    };

    const Math::Vector3f expected{
        .x = 5.0F,
        .y = 7.0F,
        .z = 9.0F,
    };

    const auto result = Math::add(vector_a, vector_b);

    EXPECT_FLOAT_EQ(result.x, expected.x);
    EXPECT_FLOAT_EQ(result.y, expected.y);
    EXPECT_FLOAT_EQ(result.z, expected.z);
}

TEST(Vector3Tests, SubtractsVectorsCorrectly) {
    const Math::Vector3f vector_a{
        .x = 5.0F,
        .y = 7.0F,
        .z = 9.0F,
    };

    const Math::Vector3f vector_b{
        .x = 1.0F,
        .y = 2.0F,
        .z = 3.0F,
    };

    const Math::Vector3f expected{
        .x = 4.0F,
        .y = 5.0F,
        .z = 6.0F,
    };

    const auto result = Math::subtract(vector_a, vector_b);

    EXPECT_FLOAT_EQ(result.x, expected.x);
    EXPECT_FLOAT_EQ(result.y, expected.y);
    EXPECT_FLOAT_EQ(result.z, expected.z);
}

TEST(Vector3Tests, ComputesDotProductCorrectly) {
    const Math::Vector3f vector_a{
        .x = 1.0F,
        .y = 2.0F,
        .z = 3.0F,
    };

    const Math::Vector3f vector_b{
        .x = 4.0F,
        .y = 5.0F,
        .z = 6.0F,
    };

    const float expected = 32.0F;

    const auto result = Math::dot(vector_a, vector_b);
    EXPECT_FLOAT_EQ(result, expected);
}

TEST(Vector3Tests, PerpendicularVectorsHaveZeroDotProduct) {
    const Math::Vector3f vector_a{
        .x = 1.0F,
        .y = 0.0F,
        .z = 0.0F,
    };

    const Math::Vector3f vector_b{
        .x = 0.0F,
        .y = 1.0F,
        .z = 0.0F,
    };

    const auto result = Math::dot(vector_a, vector_b);
    const float expected = 0.0F;

    EXPECT_FLOAT_EQ(result, expected);
}

TEST(Vector3Tests, OppositeVectorsHaveNegativeDotProduct) {
    const Math::Vector3f vector_a{
        .x = 1.0F,
        .y = 0.0F,
        .z = 0.0F,
    };

    const Math::Vector3f vector_b{
        .x = -1.0F,
        .y = 0.0F,
        .z = 0.0F,
    };

    const auto result = Math::dot(vector_a, vector_b);
    const float expected = -1.0F;

    EXPECT_FLOAT_EQ(result, expected);
}

TEST(Vector3Tests, ComputesCrossProductCorrectly) {
    const Math::Vector3f vector_a{
        .x = 1.0F,
        .y = 2.0F,
        .z = 3.0F,
    };

    const Math::Vector3f vector_b{
        .x = 4.0F,
        .y = 5.0F,
        .z = 6.0F,
    };

    const auto result = Math::cross(vector_a, vector_b);
    const Math::Vector3f expected{
        .x = -3.0F,
        .y = 6.0F,
        .z = -3.0F,
    };

    EXPECT_FLOAT_EQ(result.x, expected.x);
    EXPECT_FLOAT_EQ(result.y, expected.y);
    EXPECT_FLOAT_EQ(result.z, expected.z);
}

TEST(Vector3Tests, ReversingOperandsNegatesCrossProduct) {
    const Math::Vector3f vector_a{
        .x = 1.0F,
        .y = 2.0F,
        .z = 3.0F,
    };

    const Math::Vector3f vector_b{
        .x = 4.0F,
        .y = 5.0F,
        .z = 6.0F,
    };

    const auto result = Math::cross(vector_b, vector_a);
    const Math::Vector3f expected{
        .x = 3.0F,
        .y = -6.0F,
        .z = 3.0F,
    };

    EXPECT_FLOAT_EQ(result.x, expected.x);
    EXPECT_FLOAT_EQ(result.y, expected.y);
    EXPECT_FLOAT_EQ(result.z, expected.z);
}

TEST(Vector3Tests, ComputesSquaredLengthCorrectly) {
    const Math::Vector3f vector{
        .x = 3.0F,
        .y = 4.0F,
        .z = 0.0F,
    };

    const auto result = Math::lengthSquared(vector);
    const float expected = 25.0F;
    EXPECT_FLOAT_EQ(result, expected);
}

TEST(Vector3Tests, ComputesLengthCorrectly) {
    const Math::Vector3f vector{
        .x = 3.0F,
        .y = 4.0F,
        .z = 0.0F,
    };

    const auto result = Math::length(vector);
    const float expected = 5.0F;
    EXPECT_FLOAT_EQ(result, expected);
}

TEST(Vector3Tests, NormalizesVectorCorrectly) {
    const Math::Vector3f vector{
        .x = 3.0F,
        .y = 4.0F,
        .z = 0.0F,
    };

    const auto result = Math::normalize(vector);
    const Math::Vector3f expected{
        .x = 0.6F,
        .y = 0.8F,
        .z = 0.0F,
    };

    EXPECT_FLOAT_EQ(result.x, expected.x);
    EXPECT_FLOAT_EQ(result.y, expected.y);
    EXPECT_FLOAT_EQ(result.z, expected.z);
}

TEST(Vector3Tests, NormalizingZeroVectorTriggersPrecondition) {
    const Math::Vector3f vector{
        .x = 0.0F,
        .y = 0.0F,
        .z = 0.0F,
    };

    EXPECT_DEATH(static_cast<void>(Math::normalize(vector)),
                 "Cannot normalize a vector with an invalid magnitude");
}

TEST(Vector3Tests, ScalesVectorCorrectly) {
    const Math::Vector3f vector{
        .x = 1.0F,
        .y = 2.0F,
        .z = 3.0F,
    };

    const float multiplier = 2.0F;

    const auto result = Math::scale(vector, multiplier);
    const Math::Vector3f expected{
        .x = 2.0F,
        .y = 4.0F,
        .z = 6.0F,
    };

    EXPECT_FLOAT_EQ(result.x, expected.x);
    EXPECT_FLOAT_EQ(result.y, expected.y);
    EXPECT_FLOAT_EQ(result.z, expected.z);
}
