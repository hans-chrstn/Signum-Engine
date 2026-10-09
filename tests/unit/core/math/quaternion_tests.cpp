
#include "engine/core/math/quaternion.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <gtest/gtest.h>
#include <limits>
#include <numbers>

namespace Math = SNE::Engine::Core::Math;

namespace {
    constexpr float kTolerance = 1.0e-5F;
    constexpr float kZero = 0.0F;
    constexpr float kOne = 1.0F;
    constexpr float kScaleFactor = 2.0F;
    constexpr float kNegativeScaleFactor = -1.0F;
    constexpr float kMinimumRotationOrderDifference = 0.5F;

    constexpr float kRightAngle = std::numbers::pi_v<float> / 2.0F;
    constexpr float kThirdPiAngle = std::numbers::pi_v<float> / 3.0F;

    constexpr Math::Quaternionf kFirstQuaternion{
        .x = 1.0F,
        .y = 2.0F,
        .z = 3.0F,
        .w = 4.0F,
    };

    constexpr Math::Quaternionf kSecondQuaternion{
        .x = 4.0F,
        .y = 3.0F,
        .z = 2.0F,
        .w = 1.0F,
    };

    constexpr Math::Quaternionf kExpectedSum{
        .x = 5.0F,
        .y = 5.0F,
        .z = 5.0F,
        .w = 5.0F,
    };

    constexpr Math::Quaternionf kExpectedDifference{
        .x = -3.0F,
        .y = -1.0F,
        .z = 1.0F,
        .w = 3.0F,
    };

    constexpr Math::Quaternionf kExpectedScaledQuaternion{
        .x = 2.0F,
        .y = 4.0F,
        .z = 6.0F,
        .w = 8.0F,
    };

    constexpr Math::Quaternionf kExpectedHamiltonProduct{
        .x = 12.0F,
        .y = 24.0F,
        .z = 6.0F,
        .w = -12.0F,
    };

    constexpr Math::Quaternionf kNormalizableQuaternion{
        .x = 0.0F,
        .y = 3.0F,
        .z = 0.0F,
        .w = 4.0F,
    };

    constexpr Math::Quaternionf kExpectedNormalizedQuaternion{
        .x = 0.0F,
        .y = 0.6F,
        .z = 0.0F,
        .w = 0.8F,
    };

    constexpr float kExpectedDotProduct = 20.0F;
    constexpr float kExpectedSquaredLength = 30.0F;
    constexpr float kExpectedNormalizableSquaredLength = 25.0F;
    constexpr float kExpectedNormalizableLength = 5.0F;

    constexpr Math::Vector3f kXAxis{
        .x = 1.0F,
        .y = 0.0F,
        .z = 0.0F,
    };

    constexpr Math::Vector3f kYAxis{
        .x = 0.0F,
        .y = 1.0F,
        .z = 0.0F,
    };

    constexpr Math::Vector3f kZAxis{
        .x = 0.0F,
        .y = 0.0F,
        .z = 1.0F,
    };

    constexpr Math::Vector3f kScaledZAxis{
        .x = 0.0F,
        .y = 0.0F,
        .z = 5.0F,
    };

    [[nodiscard]] constexpr auto
    quaternionComponentsEqual(Math::Quaternionf actual,
                              Math::Quaternionf expected) -> bool {
        return actual.x == expected.x && actual.y == expected.y &&
               actual.z == expected.z && actual.w == expected.w;
    }

    auto expectQuaternionsNear(Math::Quaternionf actual,
                               Math::Quaternionf expected) -> void {
        EXPECT_NEAR(actual.x, expected.x, kTolerance);
        EXPECT_NEAR(actual.y, expected.y, kTolerance);
        EXPECT_NEAR(actual.z, expected.z, kTolerance);
        EXPECT_NEAR(actual.w, expected.w, kTolerance);
    }

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
} // namespace

// Compile-time tests

static_assert(quaternionComponentsEqual(Math::quaternionIdentity(),
                                        Math::Quaternionf{
                                            .x = kZero,
                                            .y = kZero,
                                            .z = kZero,
                                            .w = kOne,
                                        }));

static_assert(quaternionComponentsEqual(Math::add(kFirstQuaternion,
                                                  kSecondQuaternion),
                                        kExpectedSum));

static_assert(quaternionComponentsEqual(Math::subtract(kFirstQuaternion,
                                                       kSecondQuaternion),
                                        kExpectedDifference));

static_assert(quaternionComponentsEqual(Math::scale(kFirstQuaternion,
                                                    kScaleFactor),
                                        kExpectedScaledQuaternion));

static_assert(Math::dot(kFirstQuaternion, kSecondQuaternion) ==
              kExpectedDotProduct);

static_assert(Math::lengthSquared(kFirstQuaternion) == kExpectedSquaredLength);

static_assert(quaternionComponentsEqual(Math::multiply(kFirstQuaternion,
                                                       kSecondQuaternion),
                                        kExpectedHamiltonProduct));

static_assert(quaternionComponentsEqual(
    Math::multiply(kFirstQuaternion, Math::quaternionIdentity()),
    kFirstQuaternion));

static_assert(quaternionComponentsEqual(
    Math::multiply(Math::quaternionIdentity(), kFirstQuaternion),
    kFirstQuaternion));

// Runtime tests

TEST(QuaternionTests, DefaultInitializationProducesZeroQuaternion) {
    const Math::Quaternionf quaternion{};

    const Math::Quaternionf expected{
        .x = kZero,
        .y = kZero,
        .z = kZero,
        .w = kZero,
    };

    expectQuaternionsNear(quaternion, expected);
}

TEST(QuaternionTests, IdentityHasExpectedComponents) {
    const Math::Quaternionf expected{
        .x = kZero,
        .y = kZero,
        .z = kZero,
        .w = kOne,
    };

    const auto result = Math::quaternionIdentity();

    expectQuaternionsNear(result, expected);
}

TEST(QuaternionTests, AddsAllQuaternionComponents) {
    const auto result = Math::add(kFirstQuaternion, kSecondQuaternion);

    expectQuaternionsNear(result, kExpectedSum);
}

TEST(QuaternionTests, SubtractsAllQuaternionComponents) {
    const auto result = Math::subtract(kFirstQuaternion, kSecondQuaternion);

    expectQuaternionsNear(result, kExpectedDifference);
}

TEST(QuaternionTests, ComputesDotProductCorrectly) {
    const float result = Math::dot(kFirstQuaternion, kSecondQuaternion);

    EXPECT_FLOAT_EQ(result, kExpectedDotProduct);
}

TEST(QuaternionTests, ComputesSquaredLengthCorrectly) {
    const float result = Math::lengthSquared(kFirstQuaternion);

    EXPECT_FLOAT_EQ(result, kExpectedSquaredLength);
}

TEST(QuaternionTests, ComputesLengthCorrectly) {
    const float result = Math::length(kNormalizableQuaternion);

    EXPECT_FLOAT_EQ(result, kExpectedNormalizableLength);
}

TEST(QuaternionTests, ScalesAllQuaternionComponents) {
    const auto result = Math::scale(kFirstQuaternion, kScaleFactor);

    expectQuaternionsNear(result, kExpectedScaledQuaternion);
}

TEST(QuaternionTests, NormalizesQuaternion) {
    const auto result = Math::normalize(kNormalizableQuaternion);

    expectQuaternionsNear(result, kExpectedNormalizedQuaternion);

    EXPECT_NEAR(Math::length(result), kOne, kTolerance);
    EXPECT_FLOAT_EQ(Math::lengthSquared(kNormalizableQuaternion),
                    kExpectedNormalizableSquaredLength);
}

TEST(QuaternionTests, RejectsZeroQuaternionNormalization) {
    const Math::Quaternionf quaternion{};

    EXPECT_DEATH(static_cast<void>(Math::normalize(quaternion)),
                 "Cannot normalize a quaternion with an invalid magnitude");
}

TEST(QuaternionTests, RejectsNonFiniteQuaternionNormalization) {
    const Math::Quaternionf quaternion{
        .x = kZero,
        .y = kZero,
        .z = kZero,
        .w = std::numeric_limits<float>::infinity(),
    };

    EXPECT_DEATH(static_cast<void>(Math::normalize(quaternion)),
                 "Cannot normalize a quaternion with an invalid magnitude");
}

TEST(QuaternionTests, HamiltonProductMatchesKnownResult) {
    const auto result = Math::multiply(kFirstQuaternion, kSecondQuaternion);

    expectQuaternionsNear(result, kExpectedHamiltonProduct);
}

TEST(QuaternionTests, IdentityMultiplicationPreservesQuaternion) {
    const auto right_identity =
        Math::multiply(kFirstQuaternion, Math::quaternionIdentity());

    const auto left_identity =
        Math::multiply(Math::quaternionIdentity(), kFirstQuaternion);

    expectQuaternionsNear(right_identity, kFirstQuaternion);
    expectQuaternionsNear(left_identity, kFirstQuaternion);
}

TEST(QuaternionTests, IdentityConvertsToIdentityMatrix) {
    const auto result = Math::toRotationMatrix(Math::quaternionIdentity());

    expectMatricesNear(result, Math::identity());
}

TEST(QuaternionTests, AxisAngleMatchesXYZRotationMatrices) {
    const std::array axes{
        kXAxis,
        kYAxis,
        kZAxis,
    };

    const std::array expected_matrices{
        Math::rotateX(kRightAngle),
        Math::rotateY(kRightAngle),
        Math::rotateZ(kRightAngle),
    };

    for (std::size_t index{}; index < axes.size(); ++index) {
        const auto quaternion = Math::fromAxisAngle(axes[index], kRightAngle);

        const auto result = Math::toRotationMatrix(quaternion);

        expectMatricesNear(result, expected_matrices[index]);
    }
}

TEST(QuaternionTests, AxisAngleNormalizesItsAxis) {
    const auto first = Math::fromAxisAngle(kZAxis, kThirdPiAngle);

    const auto second = Math::fromAxisAngle(kScaledZAxis, kThirdPiAngle);

    expectQuaternionsNear(first, second);
}

TEST(QuaternionTests, AxisAngleProducesUnitQuaternion) {
    const auto result = Math::fromAxisAngle(kYAxis, kThirdPiAngle);

    EXPECT_NEAR(Math::length(result), kOne, kTolerance);
}

TEST(QuaternionTests, HamiltonProductPreservesRotationOrder) {
    const auto rotation_x = Math::fromAxisAngle(kXAxis, kRightAngle);

    const auto rotation_y = Math::fromAxisAngle(kYAxis, kRightAngle);

    const auto combined = Math::multiply(rotation_y, rotation_x);

    const auto expected_matrix =
        Math::multiply(Math::rotateY(kRightAngle), Math::rotateX(kRightAngle));

    expectMatricesNear(Math::toRotationMatrix(combined), expected_matrix);

    const auto reversed = Math::multiply(rotation_x, rotation_y);

    EXPECT_GT(std::abs(combined.z - reversed.z),
              kMinimumRotationOrderDifference);
}

TEST(QuaternionTests, NegatedQuaternionRepresentsSameRotation) {
    const auto quaternion = Math::fromAxisAngle(kYAxis, kThirdPiAngle);

    const auto negated = Math::scale(quaternion, kNegativeScaleFactor);

    expectMatricesNear(Math::toRotationMatrix(quaternion),
                       Math::toRotationMatrix(negated));
}

TEST(QuaternionTests, NonUnitQuaternionProducesSameRotationMatrix) {
    const auto quaternion = Math::fromAxisAngle(kXAxis, kThirdPiAngle);

    const auto scaled = Math::scale(quaternion, kScaleFactor);

    expectMatricesNear(Math::toRotationMatrix(quaternion),
                       Math::toRotationMatrix(scaled));
}

TEST(QuaternionTests, RejectsZeroRotationAxis) {
    const Math::Vector3f invalid_axis{};

    EXPECT_DEATH(
        static_cast<void>(Math::fromAxisAngle(invalid_axis, kRightAngle)),
        "invalid magnitude");
}

TEST(QuaternionTests, RejectsInfiniteAxisAngle) {
    const float invalid_angle = std::numeric_limits<float>::infinity();

    EXPECT_DEATH(static_cast<void>(Math::fromAxisAngle(kZAxis, invalid_angle)),
                 "Cannot create a quaternion from a non-finite angle");
}

TEST(QuaternionTests, RejectsNaNAxisAngle) {
    const float invalid_angle = std::numeric_limits<float>::quiet_NaN();

    EXPECT_DEATH(static_cast<void>(Math::fromAxisAngle(kZAxis, invalid_angle)),
                 "Cannot create a quaternion from a non-finite angle");
}

TEST(QuaternionTests, RejectsZeroQuaternionMatrixConversion) {
    const Math::Quaternionf quaternion{};

    EXPECT_DEATH(static_cast<void>(Math::toRotationMatrix(quaternion)),
                 "Cannot normalize a quaternion with an invalid magnitude");
}
