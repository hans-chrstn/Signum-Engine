#include "engine/core/math/vector2.hpp"
#include <gtest/gtest.h>

namespace Math = SNE::Engine::Core::Math;

static_assert([] -> bool {
    constexpr Math::Vector2f first{.x = 1.0F};
    constexpr Math::Vector2f second{.y = 1.0F};

    constexpr auto result = Math::add(first, second);

    return result.x == 1.0F && result.y == 1.0F;
}());

static_assert([] -> bool {
    constexpr Math::Vector2f first{.x = 1.0F};
    constexpr Math::Vector2f second{.y = 1.0F};

    constexpr auto result = Math::subtract(first, second);

    return result.x == 1.0F && result.y == -1.0F;
}());

static_assert([] -> bool {
    constexpr Math::Vector2f vector{
        .x = 3.0F,
        .y = 4.0F,
    };

    constexpr float result = Math::lengthSquared(vector);
    constexpr float expected = 25.0F;

    return result == expected;
}());

static_assert([] -> bool {
    constexpr Math::Vector2f vector{
        .x = 1.0F,
        .y = 2.0F,
    };

    constexpr float multiplier = 2.0F;

    constexpr Math::Vector2f expected{
        .x = 2.0F,
        .y = 4.0F,
    };

    constexpr Math::Vector2f result = Math::scale(vector, multiplier);

    return result.x == expected.x && result.y == expected.y;
}());

TEST(Vector2Tests, AddsVectorsCorrectly) {
    const Math::Vector2f vector_a{
        .x = 1.0F,
        .y = 2.0F,
    };

    const Math::Vector2f vector_b{
        .x = 4.0F,
        .y = 5.0F,
    };

    const Math::Vector2f expected{
        .x = 5.0F,
        .y = 7.0F,
    };

    const auto result = Math::add(vector_a, vector_b);

    EXPECT_FLOAT_EQ(result.x, expected.x);
    EXPECT_FLOAT_EQ(result.y, expected.y);
}

TEST(Vector2Tests, SubtractsVectorsCorrectly) {
    const Math::Vector2f vector_a{
        .x = 5.0F,
        .y = 7.0F,
    };

    const Math::Vector2f vector_b{
        .x = 1.0F,
        .y = 2.0F,
    };

    const Math::Vector2f expected{
        .x = 4.0F,
        .y = 5.0F,
    };

    const auto result = Math::subtract(vector_a, vector_b);

    EXPECT_FLOAT_EQ(result.x, expected.x);
    EXPECT_FLOAT_EQ(result.y, expected.y);
}

TEST(Vector2Tests, ComputesDotProductCorrectly) {
    const Math::Vector2f vector_a{
        .x = 1.0F,
        .y = 2.0F,
    };

    const Math::Vector2f vector_b{
        .x = 4.0F,
        .y = 5.0F,
    };

    const float expected = 14.0F;

    const auto result = Math::dot(vector_a, vector_b);
    EXPECT_FLOAT_EQ(result, expected);
}

TEST(Vector2Tests, PerpendicularVectorsHaveZeroDotProduct) {
    const Math::Vector2f vector_a{
        .x = 1.0F,
        .y = 0.0F,
    };

    const Math::Vector2f vector_b{
        .x = 0.0F,
        .y = 1.0F,
    };

    const auto result = Math::dot(vector_a, vector_b);
    const float expected = 0.0F;

    EXPECT_FLOAT_EQ(result, expected);
}

TEST(Vector2Tests, OppositeVectorsHaveNegativeDotProduct) {
    const Math::Vector2f vector_a{
        .x = 1.0F,
        .y = 0.0F,
    };

    const Math::Vector2f vector_b{
        .x = -1.0F,
        .y = 0.0F,
    };

    const auto result = Math::dot(vector_a, vector_b);
    const float expected = -1.0F;

    EXPECT_FLOAT_EQ(result, expected);
}

TEST(Vector2Tests, ComputesSquaredLengthCorrectly) {
    const Math::Vector2f vector{
        .x = 3.0F,
        .y = 4.0F,
    };

    const auto result = Math::lengthSquared(vector);
    const float expected = 25.0F;
    EXPECT_FLOAT_EQ(result, expected);
}

TEST(Vector2Tests, ComputesLengthCorrectly) {
    const Math::Vector2f vector{
        .x = 3.0F,
        .y = 4.0F,
    };

    const auto result = Math::length(vector);
    const float expected = 5.0F;
    EXPECT_FLOAT_EQ(result, expected);
}

TEST(Vector2Tests, NormalizesVectorCorrectly) {
    const Math::Vector2f vector{
        .x = 3.0F,
        .y = 4.0F,
    };

    const auto result = Math::normalize(vector);
    const Math::Vector2f expected{
        .x = 0.6F,
        .y = 0.8F,
    };

    EXPECT_FLOAT_EQ(result.x, expected.x);
    EXPECT_FLOAT_EQ(result.y, expected.y);
}

TEST(Vector2Tests, NormalizingZeroVectorTriggersPrecondition) {
    const Math::Vector2f vector{
        .x = 0.0F,
        .y = 0.0F,
    };

    EXPECT_DEATH(static_cast<void>(Math::normalize(vector)),
                 "Cannot normalize a vector with an invalid magnitude");
}

TEST(Vector2Tests, ScalesVectorCorrectly) {
    const Math::Vector2f vector{
        .x = 1.0F,
        .y = 2.0F,
    };

    const float multiplier = 2.0F;

    const auto result = Math::scale(vector, multiplier);
    const Math::Vector2f expected{
        .x = 2.0F,
        .y = 4.0F,
    };

    EXPECT_FLOAT_EQ(result.x, expected.x);
    EXPECT_FLOAT_EQ(result.y, expected.y);
}
