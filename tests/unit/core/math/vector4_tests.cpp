
#include "engine/core/math/vector4.hpp"
#include <gtest/gtest.h>

namespace Math = SNE::Engine::Core::Math;

static_assert([] -> bool {
    constexpr Math::Vector4f first{
        .x = 1.0F,
        .y = 2.0F,
        .z = 3.0F,
        .w = 4.0F,
    };

    constexpr Math::Vector4f second{
        .x = 4.0F,
        .y = 5.0F,
        .z = 6.0F,
        .w = 7.0F,
    };

    constexpr Math::Vector4f expected{
        .x = 5.0F,
        .y = 7.0F,
        .z = 9.0F,
        .w = 11.0F,
    };

    constexpr auto result = Math::add(first, second);

    return result.x == expected.x && result.y == expected.y &&
           result.z == expected.z && result.w == expected.w;
}());

static_assert([] -> bool {
    constexpr Math::Vector4f first{
        .x = 5.0F,
        .y = 7.0F,
        .z = 9.0F,
        .w = 11.0F,
    };

    constexpr Math::Vector4f second{
        .x = 1.0F,
        .y = 2.0F,
        .z = 3.0F,
        .w = 4.0F,
    };

    constexpr Math::Vector4f expected{
        .x = 4.0F,
        .y = 5.0F,
        .z = 6.0F,
        .w = 7.0F,
    };

    constexpr auto result = Math::subtract(first, second);

    return result.x == expected.x && result.y == expected.y &&
           result.z == expected.z && result.w == expected.w;
}());

static_assert([] -> bool {
    constexpr Math::Vector4f first{
        .x = 1.0F,
        .y = 2.0F,
        .z = 3.0F,
        .w = 4.0F,
    };

    constexpr Math::Vector4f second{
        .x = 4.0F,
        .y = 5.0F,
        .z = 6.0F,
        .w = 7.0F,
    };

    constexpr float result = Math::dot(first, second);
    constexpr float expected = 60.0F;

    return result == expected;
}());

static_assert([] -> bool {
    constexpr Math::Vector4f vector{
        .x = 1.0F,
        .y = 2.0F,
        .z = 2.0F,
        .w = 4.0F,
    };

    constexpr float result = Math::lengthSquared(vector);
    constexpr float expected = 25.0F;

    return result == expected;
}());

static_assert([] -> bool {
    constexpr Math::Vector4f vector{
        .x = 1.0F,
        .y = 2.0F,
        .z = 3.0F,
        .w = 4.0F,
    };

    constexpr float multiplier = 2.0F;

    constexpr Math::Vector4f expected{
        .x = 2.0F,
        .y = 4.0F,
        .z = 6.0F,
        .w = 8.0F,
    };

    constexpr Math::Vector4f result = Math::scale(vector, multiplier);

    return result.x == expected.x && result.y == expected.y &&
           result.z == expected.z && result.w == expected.w;
}());

TEST(Vector4Tests, AddsVectorsCorrectly) {
    const Math::Vector4f vector_a{
        .x = 1.0F,
        .y = 2.0F,
        .z = 3.0F,
        .w = 4.0F,
    };

    const Math::Vector4f vector_b{
        .x = 4.0F,
        .y = 5.0F,
        .z = 6.0F,
        .w = 7.0F,
    };

    const Math::Vector4f expected{
        .x = 5.0F,
        .y = 7.0F,
        .z = 9.0F,
        .w = 11.0F,
    };

    const auto result = Math::add(vector_a, vector_b);

    EXPECT_FLOAT_EQ(result.x, expected.x);
    EXPECT_FLOAT_EQ(result.y, expected.y);
    EXPECT_FLOAT_EQ(result.z, expected.z);
    EXPECT_FLOAT_EQ(result.w, expected.w);
}

TEST(Vector4Tests, SubtractsVectorsCorrectly) {
    const Math::Vector4f vector_a{
        .x = 5.0F,
        .y = 7.0F,
        .z = 9.0F,
        .w = 11.0F,
    };

    const Math::Vector4f vector_b{
        .x = 1.0F,
        .y = 2.0F,
        .z = 3.0F,
        .w = 4.0F,
    };

    const Math::Vector4f expected{
        .x = 4.0F,
        .y = 5.0F,
        .z = 6.0F,
        .w = 7.0F,
    };

    const auto result = Math::subtract(vector_a, vector_b);

    EXPECT_FLOAT_EQ(result.x, expected.x);
    EXPECT_FLOAT_EQ(result.y, expected.y);
    EXPECT_FLOAT_EQ(result.z, expected.z);
    EXPECT_FLOAT_EQ(result.w, expected.w);
}

TEST(Vector4Tests, ComputesDotProductCorrectly) {
    const Math::Vector4f vector_a{
        .x = 1.0F,
        .y = 2.0F,
        .z = 3.0F,
        .w = 4.0F,
    };

    const Math::Vector4f vector_b{
        .x = 4.0F,
        .y = 5.0F,
        .z = 6.0F,
        .w = 7.0F,
    };

    const float expected = 60.0F;

    const auto result = Math::dot(vector_a, vector_b);

    EXPECT_FLOAT_EQ(result, expected);
}

TEST(Vector4Tests, PerpendicularVectorsHaveZeroDotProduct) {
    const Math::Vector4f vector_a{
        .x = 1.0F,
        .y = 0.0F,
        .z = 0.0F,
        .w = 0.0F,
    };

    const Math::Vector4f vector_b{
        .x = 0.0F,
        .y = 0.0F,
        .z = 0.0F,
        .w = 1.0F,
    };

    const auto result = Math::dot(vector_a, vector_b);
    const float expected = 0.0F;

    EXPECT_FLOAT_EQ(result, expected);
}

TEST(Vector4Tests, OppositeVectorsHaveNegativeDotProduct) {
    const Math::Vector4f vector_a{
        .x = 0.0F,
        .y = 0.0F,
        .z = 0.0F,
        .w = 1.0F,
    };

    const Math::Vector4f vector_b{
        .x = 0.0F,
        .y = 0.0F,
        .z = 0.0F,
        .w = -1.0F,
    };

    const auto result = Math::dot(vector_a, vector_b);
    const float expected = -1.0F;

    EXPECT_FLOAT_EQ(result, expected);
}

TEST(Vector4Tests, ComputesSquaredLengthCorrectly) {
    const Math::Vector4f vector{
        .x = 1.0F,
        .y = 2.0F,
        .z = 2.0F,
        .w = 4.0F,
    };

    const auto result = Math::lengthSquared(vector);
    const float expected = 25.0F;

    EXPECT_FLOAT_EQ(result, expected);
}

TEST(Vector4Tests, ComputesLengthCorrectly) {
    const Math::Vector4f vector{
        .x = 1.0F,
        .y = 2.0F,
        .z = 2.0F,
        .w = 4.0F,
    };

    const auto result = Math::length(vector);
    const float expected = 5.0F;

    EXPECT_FLOAT_EQ(result, expected);
}

TEST(Vector4Tests, NormalizesVectorCorrectly) {
    const Math::Vector4f vector{
        .x = 0.0F,
        .y = 3.0F,
        .z = 0.0F,
        .w = 4.0F,
    };

    const auto result = Math::normalize(vector);

    const Math::Vector4f expected{
        .x = 0.0F,
        .y = 0.6F,
        .z = 0.0F,
        .w = 0.8F,
    };

    EXPECT_FLOAT_EQ(result.x, expected.x);
    EXPECT_FLOAT_EQ(result.y, expected.y);
    EXPECT_FLOAT_EQ(result.z, expected.z);
    EXPECT_FLOAT_EQ(result.w, expected.w);
}

TEST(Vector4Tests, NormalizingZeroVectorTriggersPrecondition) {
    const Math::Vector4f vector{
        .x = 0.0F,
        .y = 0.0F,
        .z = 0.0F,
        .w = 0.0F,
    };

    EXPECT_DEATH(static_cast<void>(Math::normalize(vector)),
                 "Cannot normalize a vector with an invalid magnitude");
}

TEST(Vector4Tests, ScalesVectorCorrectly) {
    const Math::Vector4f vector{
        .x = 1.0F,
        .y = 2.0F,
        .z = 3.0F,
        .w = 4.0F,
    };

    const float multiplier = 2.0F;

    const auto result = Math::scale(vector, multiplier);

    const Math::Vector4f expected{
        .x = 2.0F,
        .y = 4.0F,
        .z = 6.0F,
        .w = 8.0F,
    };

    EXPECT_FLOAT_EQ(result.x, expected.x);
    EXPECT_FLOAT_EQ(result.y, expected.y);
    EXPECT_FLOAT_EQ(result.z, expected.z);
    EXPECT_FLOAT_EQ(result.w, expected.w);
}
