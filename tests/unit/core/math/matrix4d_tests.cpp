#include "engine/core/math/matrix4d.hpp"
#include "engine/core/math/vector3d.hpp"
#include <cmath>
#include <cstddef>
#include <gtest/gtest.h>
#include <limits>
#include <numbers>

namespace Math = SNE::Engine::Core::Math;

// Compile-time tests
static_assert([] -> bool {
    constexpr auto matrix = Math::matrix4dIdentity();

    for (std::size_t column{}; column < Math::kMatrix4dDimension; ++column) {
        for (std::size_t row{}; row < Math::kMatrix4dDimension; ++row) {
            const std::size_t index = (column * Math::kMatrix4dDimension) + row;

            const double expected = (row == column) ? 1.0 : 0.0;

            if (matrix.elements[index] != expected) {
                return false;
            }
        }
    }

    return true;
}());

static_assert([] -> bool {
    constexpr std::size_t kTranslationRow = 1U;
    constexpr std::size_t kTranslationColumn = Math::kMatrix4dDimension - 1U;

    constexpr std::size_t kExpectedStorageIndex = 13U;
    constexpr double kExpectedTranslation = 5.0;

    Math::Matrix4d matrix = Math::matrix4dIdentity();

    matrix.at(kTranslationRow, kTranslationColumn) = kExpectedTranslation;

    return matrix.at(kTranslationRow, kTranslationColumn) ==
               kExpectedTranslation &&
           matrix.elements[kExpectedStorageIndex] == kExpectedTranslation;
}());

static_assert([] -> bool {
    constexpr Math::Matrix4d matrix = Math::matrix4dIdentity();

    return matrix.at(0U, 0U) == 1.0;
}());

static_assert(Math::translate(0.0, 0.0, 0.0).elements ==
              Math::matrix4dIdentity().elements);

static_assert([] -> bool {
    constexpr double kScaleX = 2.0;
    constexpr double kScaleY = 3.0;
    constexpr double kScaleZ = 4.0;

    constexpr auto matrix = Math::scale(kScaleX, kScaleY, kScaleZ);
    constexpr std::size_t kLastIndex = Math::kMatrix4dDimension - 1U;

    return matrix.at(0U, 0U) == kScaleX && matrix.at(1U, 1U) == kScaleY &&
           matrix.at(2U, 2U) == kScaleZ &&
           matrix.at(kLastIndex, kLastIndex) == 1.0;
}());

TEST(Matrix4dTests, DefaultInitializationProducesZeroMatrix) {
    const Math::Matrix4d matrix{};

    for (const double element : matrix.elements) {
        EXPECT_DOUBLE_EQ(element, 0.0);
    }
}

TEST(Matrix4dTests, IdentityMultipliedByIdentityIsIdentity) {
    const Math::Matrix4d identity = Math::matrix4dIdentity();

    const auto result = Math::multiply(identity, identity);

    for (std::size_t i{}; i < identity.elements.size(); ++i) {
        EXPECT_DOUBLE_EQ(result.elements[i], identity.elements[i]);
    }
}

TEST(Matrix4dTests, MultipliesTranslationAndScaleCorrectly) {
    // clang-format off
    const Math::Matrix4d translated{
        .elements = {
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            3.0, 2.0, 0.0, 1.0,
        },
    };
    const Math::Matrix4d scaled{
        .elements = {
            2.0, 0.0, 0.0, 0.0,
            0.0, 2.0, 0.0, 0.0,
            0.0, 0.0, 2.0, 0.0,
            0.0, 0.0, 0.0, 1.0,
        },
    };
    const Math::Matrix4d expected{
        .elements = {
            2.0, 0.0, 0.0, 0.0,
            0.0, 2.0, 0.0, 0.0,
            0.0, 0.0, 2.0, 0.0,
            6.0, 4.0, 0.0, 1.0,
        },
    };
    // clang-format on

    // Translate first, then scale.
    const auto result = Math::multiply(scaled, translated);
    for (std::size_t i{}; i < result.elements.size(); ++i) {
        EXPECT_DOUBLE_EQ(result.elements[i], expected.elements[i]);
    }
}

TEST(Matrix4dTests, MultipliesScaleThenTranslationCorrectly) {
    // clang-format off
    const Math::Matrix4d translated{
        .elements = {
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            3.0, 2.0, 0.0, 1.0,
        },
    };
    const Math::Matrix4d scaled{
        .elements = {
            2.0, 0.0, 0.0, 0.0,
            0.0, 2.0, 0.0, 0.0,
            0.0, 0.0, 2.0, 0.0,
            0.0, 0.0, 0.0, 1.0,
        },
    };
    const Math::Matrix4d expected{
        .elements = {
            2.0, 0.0, 0.0, 0.0,
            0.0, 2.0, 0.0, 0.0,
            0.0, 0.0, 2.0, 0.0,
            3.0, 2.0, 0.0, 1.0,
        },
    };
    // clang-format on

    const auto result = Math::multiply(translated, scaled);

    for (std::size_t i{}; i < result.elements.size(); ++i) {
        EXPECT_DOUBLE_EQ(result.elements[i], expected.elements[i]);
    }
}

TEST(Matrix4dTests, IdentityPreservesNonIdentityMatrix) {
    // clang-format off
    const Math::Matrix4d translated{
        .elements = {
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            3.0, 2.0, 0.0, 1.0,
        },
    };
    // clang-format on

    const Math::Matrix4d identity = Math::matrix4dIdentity();

    const auto translate_first = Math::multiply(translated, identity);
    const auto identity_first = Math::multiply(identity, translated);

    for (std::size_t i{}; i < translated.elements.size(); ++i) {
        EXPECT_DOUBLE_EQ(translate_first.elements[i], translated.elements[i]);
        EXPECT_DOUBLE_EQ(identity_first.elements[i], translated.elements[i]);
    }
}

TEST(Matrix4dTests, MultipliesNonUniformScaleAndTranslationCorrectly) {
    // clang-format off
    const Math::Matrix4d translated{
        .elements = {
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            3.0, 2.0, 0.0, 1.0,
        },
    };
    const Math::Matrix4d non_uniform{
        .elements = {
            2.0, 0.0, 0.0, 0.0,
            0.0, 3.0, 0.0, 0.0,
            0.0, 0.0, 4.0, 0.0,
            0.0, 0.0, 0.0, 1.0,
        },
    };
    const Math::Matrix4d expected_translate_then_scale{
        .elements = {
            2.0, 0.0, 0.0, 0.0,
            0.0, 3.0, 0.0, 0.0,
            0.0, 0.0, 4.0, 0.0,
            6.0, 6.0, 0.0, 1.0,
        },
    };
    const Math::Matrix4d expected_scale_then_translate{
        .elements = {
            2.0, 0.0, 0.0, 0.0,
            0.0, 3.0, 0.0, 0.0,
            0.0, 0.0, 4.0, 0.0,
            3.0, 2.0, 0.0, 1.0,
        },
    };
    // clang-format on

    const auto scale_then_translate = Math::multiply(translated, non_uniform);
    const auto translate_then_scale = Math::multiply(non_uniform, translated);

    for (std::size_t i{}; i < translated.elements.size(); ++i) {
        EXPECT_DOUBLE_EQ(scale_then_translate.elements[i],
                         expected_scale_then_translate.elements[i]);

        EXPECT_DOUBLE_EQ(translate_then_scale.elements[i],
                         expected_translate_then_scale.elements[i]);
    }
}

TEST(Matrix4dTests, MultipliesMatricesWithOffDiagonalValuesCorrectly) {
    // clang-format off
    const Math::Matrix4d matrix_a {
        .elements = {
            1.0, 3.0, 0.0, 0.0,
            2.0, 4.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0,
        },
    };
    const Math::Matrix4d matrix_b {
        .elements = {
            5.0, 7.0, 0.0, 0.0,
            6.0, 8.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0,
        },
    };
    const Math::Matrix4d expected {
        .elements = {
            19.0, 43.0, 0.0, 0.0,
            22.0, 50.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0,
        },
    };
    // clang-format on

    const auto result = Math::multiply(matrix_a, matrix_b);

    for (std::size_t i{}; i < result.elements.size(); ++i) {
        EXPECT_DOUBLE_EQ(result.elements[i], expected.elements[i]);
    }
}

TEST(Matrix4dTests, IdentityCreatesIdentityMatrix) {
    // clang-format off
    const Math::Matrix4d expected {
        .elements = {
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0,
        },
    };
    // clang-format on

    const auto result = Math::matrix4dIdentity();
    for (std::size_t i{}; i < result.elements.size(); ++i) {
        EXPECT_DOUBLE_EQ(result.elements[i], expected.elements[i]);
    }
}

TEST(Matrix4dTests, MutableAccessorWritesCorrectElement) {
    Math::Matrix4d matrix = Math::matrix4dIdentity();
    const double change = 5.0;
    matrix.at(1U, 3U) = change;

    const std::size_t index = 13U;
    EXPECT_DOUBLE_EQ(matrix.elements[index], change);

    const Math::Matrix4d expected = Math::matrix4dIdentity();

    for (std::size_t i{}; i < matrix.elements.size(); ++i) {
        if (i == index) {
            continue;
        }

        EXPECT_DOUBLE_EQ(matrix.elements[i], expected.elements[i]);
    }
}

TEST(Matrix4dTests, ConstAccessorReadsCorrectElement) {
    // clang-format off
    const Math::Matrix4d matrix{
        .elements = {
            19.0, 43.0, 0.0, 0.0,
            22.0, 50.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0,
        },
    };
    // clang-format on

    for (std::size_t column{}; column < Math::kMatrix4dDimension; ++column) {
        for (std::size_t row{}; row < Math::kMatrix4dDimension; ++row) {
            const std::size_t index = (column * Math::kMatrix4dDimension) + row;
            EXPECT_DOUBLE_EQ(matrix.at(row, column), matrix.elements[index]);
        }
    }
}

TEST(Matrix4dTests, ConstAccessorReadsKnownCoordinates) {
    // clang-format off
    const Math::Matrix4d matrix{
        .elements = {
            19.0, 43.0, 0.0, 0.0,
            22.0, 50.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0,
        },
    };
    // clang-format on

    EXPECT_DOUBLE_EQ(matrix.at(0U, 0U), 19.0);
    EXPECT_DOUBLE_EQ(matrix.at(1U, 0U), 43.0);
    EXPECT_DOUBLE_EQ(matrix.at(0U, 1U), 22.0);
    EXPECT_DOUBLE_EQ(matrix.at(1U, 1U), 50.0);
}

TEST(Matrix4dTests, AccessorRejectsInvalidRow) {
    Math::Matrix4d matrix = Math::matrix4dIdentity();

    EXPECT_DEATH(
        { static_cast<void>(matrix.at(Math::kMatrix4dDimension, 0U)); },
        "Matrix4d row or column is out of bounds");
}

TEST(Matrix4dTests, ConstAccessorRejectsInvalidColumn) {
    const Math::Matrix4d matrix = Math::matrix4dIdentity();

    EXPECT_DEATH(
        { static_cast<void>(matrix.at(0U, Math::kMatrix4dDimension)); },
        "Matrix4d row or column is out of bounds");
}

TEST(Matrix4dTests, TranslateCreatesCorrectMatrix) {
    const Math::Matrix4d translated = Math::translate(3.0, 2.0, 5.0);

    // clang-format off
    const Math::Matrix4d expected{
        .elements = {
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            3.0, 2.0, 5.0, 1.0,
        },
    };
    // clang-format on

    for (std::size_t i{}; i < translated.elements.size(); ++i) {
        EXPECT_DOUBLE_EQ(translated.elements[i], expected.elements[i]);
    }
}

TEST(Matrix4dTests, ScaleCreatesCorrectMatrix) {
    const auto scaled = Math::scale(2.0, 3.0, 4.0);

    // clang-format off
    const Math::Matrix4d expected {
        .elements = {
            2.0, 0.0, 0.0, 0.0,
            0.0, 3.0, 0.0, 0.0,
            0.0, 0.0, 4.0, 0.0,
            0.0, 0.0, 0.0, 1.0,
        },
    };
    // clang-format on

    for (std::size_t i{}; i < scaled.elements.size(); ++i) {
        EXPECT_DOUBLE_EQ(scaled.elements[i], expected.elements[i]);
    }
}

TEST(Matrix4dTests, RotateZCreatesCorrectMatrix) {
    constexpr double kRightAngle = std::numbers::pi_v<double> / 2.0;
    constexpr double kTolerance = 1.0e-12;

    const auto result = Math::rotateZ(kRightAngle);

    // clang-format off
    const Math::Matrix4d expected{
        .elements = {
             0.0, 1.0, 0.0, 0.0,
            -1.0, 0.0, 0.0, 0.0,
             0.0, 0.0, 1.0, 0.0,
             0.0, 0.0, 0.0, 1.0,
        },
    };
    // clang-format on

    for (std::size_t column{}; column < Math::kMatrix4dDimension; ++column) {
        for (std::size_t row{}; row < Math::kMatrix4dDimension; ++row) {
            EXPECT_NEAR(result.at(row, column), expected.at(row, column),
                        kTolerance);
        }
    }
}

TEST(Matrix4dTests, RotateXCreatesCorrectMatrix) {
    constexpr double kRightAngle = std::numbers::pi_v<double> / 2.0;
    constexpr double kTolerance = 1.0e-12;

    const auto result = Math::rotateX(kRightAngle);

    // clang-format off
    const Math::Matrix4d expected{
        .elements = {
            1.0,  0.0, 0.0, 0.0,
            0.0,  0.0, 1.0, 0.0,
            0.0, -1.0, 0.0, 0.0,
            0.0,  0.0, 0.0, 1.0,
        },
    };
    // clang-format on

    for (std::size_t column{}; column < Math::kMatrix4dDimension; ++column) {
        for (std::size_t row{}; row < Math::kMatrix4dDimension; ++row) {
            EXPECT_NEAR(result.at(row, column), expected.at(row, column),
                        kTolerance);
        }
    }
}

TEST(Matrix4dTests, RotateYCreatesCorrectMatrix) {
    constexpr double kRightAngle = std::numbers::pi_v<double> / 2.0;
    constexpr double kTolerance = 1.0e-12;

    const auto result = Math::rotateY(kRightAngle);

    // clang-format off
    const Math::Matrix4d expected{
        .elements = {
             0.0, 0.0, -1.0, 0.0,
             0.0, 1.0,  0.0, 0.0,
             1.0, 0.0,  0.0, 0.0,
             0.0, 0.0,  0.0, 1.0,
        },
    };
    // clang-format on

    for (std::size_t column{}; column < Math::kMatrix4dDimension; ++column) {
        for (std::size_t row{}; row < Math::kMatrix4dDimension; ++row) {
            EXPECT_NEAR(result.at(row, column), expected.at(row, column),
                        kTolerance);
        }
    }
}

TEST(Matrix4dTests, TransformPointAppliesTranslation) {
    const Math::Vector3d point{
        .x = 1.0,
        .y = 2.0,
        .z = 3.0,
    };

    const Math::Matrix4d matrix = Math::translate(3.0, 2.0, 5.0);

    const Math::Vector3d expected{
        .x = 4.0,
        .y = 4.0,
        .z = 8.0,
    };

    const auto result = Math::transformPoint(matrix, point);

    EXPECT_DOUBLE_EQ(result.x, expected.x);
    EXPECT_DOUBLE_EQ(result.y, expected.y);
    EXPECT_DOUBLE_EQ(result.z, expected.z);
}

TEST(Matrix4dTests, TransformDirectionIgnoresTranslation) {
    const Math::Vector3d direction{
        .x = 1.0,
        .y = 2.0,
        .z = 3.0,
    };

    const Math::Matrix4d matrix = Math::translate(3.0, 2.0, 5.0);

    const Math::Vector3d expected{
        .x = 1.0,
        .y = 2.0,
        .z = 3.0,
    };

    const auto result = Math::transformDirection(matrix, direction);

    EXPECT_DOUBLE_EQ(result.x, expected.x);
    EXPECT_DOUBLE_EQ(result.y, expected.y);
    EXPECT_DOUBLE_EQ(result.z, expected.z);
}

TEST(Matrix4dTests, TransformPointAppliesRotation) {
    constexpr double kRightAngle = std::numbers::pi_v<double> / 2.0;
    constexpr double kTolerance = 1.0e-12;

    const Math::Vector3d point{
        .x = 1.0,
        .y = 0.0,
        .z = 0.0,
    };

    const Math::Matrix4d matrix = Math::rotateZ(kRightAngle);

    const Math::Vector3d expected{
        .x = 0.0,
        .y = 1.0,
        .z = 0.0,
    };

    const auto result = Math::transformPoint(matrix, point);

    EXPECT_NEAR(result.x, expected.x, kTolerance);
    EXPECT_NEAR(result.y, expected.y, kTolerance);
    EXPECT_NEAR(result.z, expected.z, kTolerance);
}

TEST(Matrix4dTests, TransformDirectionAppliesRotation) {
    constexpr double kRightAngle = std::numbers::pi_v<double> / 2.0;
    constexpr double kTolerance = 1.0e-12;

    const Math::Vector3d direction{
        .x = 1.0,
        .y = 0.0,
        .z = 0.0,
    };

    const Math::Matrix4d matrix = Math::rotateZ(kRightAngle);

    const Math::Vector3d expected{
        .x = 0.0,
        .y = 1.0,
        .z = 0.0,
    };

    const auto result = Math::transformDirection(matrix, direction);

    EXPECT_NEAR(result.x, expected.x, kTolerance);
    EXPECT_NEAR(result.y, expected.y, kTolerance);
    EXPECT_NEAR(result.z, expected.z, kTolerance);
}

TEST(Matrix4dTests, TransformPointAppliesNonUniformScale) {
    const Math::Matrix4d matrix = Math::scale(2.0, 3.0, 4.0);

    const Math::Vector3d point{
        .x = 1.0,
        .y = 2.0,
        .z = 3.0,
    };

    const Math::Vector3d expected{
        .x = 2.0,
        .y = 6.0,
        .z = 12.0,
    };

    const auto result = Math::transformPoint(matrix, point);

    EXPECT_DOUBLE_EQ(result.x, expected.x);
    EXPECT_DOUBLE_EQ(result.y, expected.y);
    EXPECT_DOUBLE_EQ(result.z, expected.z);
}

TEST(Matrix4dTests, TransformDirectionAppliesNonUniformScale) {
    const Math::Matrix4d matrix = Math::scale(2.0, 3.0, 4.0);

    const Math::Vector3d direction{
        .x = 1.0,
        .y = 2.0,
        .z = 3.0,
    };

    const Math::Vector3d expected{
        .x = 2.0,
        .y = 6.0,
        .z = 12.0,
    };

    const auto result = Math::transformDirection(matrix, direction);

    EXPECT_DOUBLE_EQ(result.x, expected.x);
    EXPECT_DOUBLE_EQ(result.y, expected.y);
    EXPECT_DOUBLE_EQ(result.z, expected.z);
}

TEST(Matrix4dTests, TransposeSwapsRowsAndColumns) {
    constexpr std::size_t kFirstIndex = 0U;
    constexpr std::size_t kSecondIndex = 1U;
    constexpr std::size_t kThirdIndex = 2U;
    constexpr std::size_t kFourthIndex = 3U;

    constexpr double kUpperValue = 2.0;
    constexpr double kLowerValue = 3.0;
    constexpr double kTranslationValue = 7.0;

    Math::Matrix4d matrix = Math::matrix4dIdentity();

    matrix.at(kFirstIndex, kSecondIndex) = kUpperValue;
    matrix.at(kSecondIndex, kFirstIndex) = kLowerValue;
    matrix.at(kThirdIndex, kFourthIndex) = kTranslationValue;

    const Math::Matrix4d result = Math::transpose(matrix);

    EXPECT_DOUBLE_EQ(result.at(kSecondIndex, kFirstIndex), kUpperValue);
    EXPECT_DOUBLE_EQ(result.at(kFirstIndex, kSecondIndex), kLowerValue);
    EXPECT_DOUBLE_EQ(result.at(kFourthIndex, kThirdIndex), kTranslationValue);
}

TEST(Matrix4dTests, TransposingTwiceRestoresOriginalMatrix) {
    constexpr std::size_t kFirstIndex = 0U;
    constexpr std::size_t kSecondIndex = 1U;
    constexpr std::size_t kThirdIndex = 2U;

    constexpr double kFirstValue = 5.0;
    constexpr double kSecondValue = -3.0;

    Math::Matrix4d matrix = Math::matrix4dIdentity();

    matrix.at(kFirstIndex, kSecondIndex) = kFirstValue;
    matrix.at(kSecondIndex, kThirdIndex) = kSecondValue;

    const Math::Matrix4d transposed = Math::transpose(matrix);
    const Math::Matrix4d restored = Math::transpose(transposed);

    EXPECT_EQ(restored.elements, matrix.elements);
}

TEST(Matrix4dTests, ComposeTRSAppliesScaleRotationAndTranslation) {
    constexpr double kRightAngle = std::numbers::pi_v<double> / 2.0;
    constexpr double kTolerance = 1.0e-12;

    const Math::Vector3d translation{
        .x = 5.0,
        .y = -2.0,
        .z = 3.0,
    };

    const Math::Vector3d rotation_radians{
        .x = 0.0,
        .y = 0.0,
        .z = kRightAngle,
    };

    const Math::Vector3d scale_factors{
        .x = 2.0,
        .y = 3.0,
        .z = 4.0,
    };

    const Math::Vector3d point{
        .x = 1.0,
        .y = 2.0,
        .z = -1.0,
    };

    const Math::Vector3d expected{
        .x = -1.0,
        .y = 0.0,
        .z = -1.0,
    };

    const Math::Matrix4d matrix =
        Math::composeTRS(translation, rotation_radians, scale_factors);

    const Math::Vector3d result = Math::transformPoint(matrix, point);

    EXPECT_NEAR(result.x, expected.x, kTolerance);
    EXPECT_NEAR(result.y, expected.y, kTolerance);
    EXPECT_NEAR(result.z, expected.z, kTolerance);
}

TEST(Matrix4dTests, ComposeTRSWithIdentityRotationAndScaleMatchesTranslation) {
    const Math::Vector3d translation{
        .x = 3.0,
        .y = -4.0,
        .z = 5.0,
    };

    const Math::Vector3d rotation_radians{};

    const Math::Vector3d scale_factors{
        .x = 1.0,
        .y = 1.0,
        .z = 1.0,
    };

    const Math::Matrix4d result =
        Math::composeTRS(translation, rotation_radians, scale_factors);

    const Math::Matrix4d expected =
        Math::translate(translation.x, translation.y, translation.z);

    for (std::size_t index{}; index < expected.elements.size(); ++index) {
        EXPECT_DOUBLE_EQ(result.elements[index], expected.elements[index]);
    }
}

TEST(Matrix4dTests, InvertsIdentityMatrix) {
    const Math::Matrix4d matrix = Math::matrix4dIdentity();

    const auto inverse = Math::tryInverse(matrix);

    if (!inverse.has_value()) {
        FAIL() << "Expected matrix inversion to succeed";
        return;
    }

    EXPECT_EQ(inverse->elements, matrix.elements);
}

TEST(Matrix4dTests, InvertsTranslationMatrix) {
    const Math::Vector3d translation{
        .x = 3.0,
        .y = -4.0,
        .z = 5.0,
    };

    const Math::Vector3d inverse_translation{
        .x = -3.0,
        .y = 4.0,
        .z = -5.0,
    };

    const Math::Matrix4d matrix =
        Math::translate(translation.x, translation.y, translation.z);

    const Math::Matrix4d expected = Math::translate(
        inverse_translation.x, inverse_translation.y, inverse_translation.z);

    const auto inverse = Math::tryInverse(matrix);

    if (!inverse.has_value()) {
        FAIL() << "Expected matrix inversion to succeed";
        return;
    }

    for (std::size_t index{}; index < expected.elements.size(); ++index) {
        EXPECT_DOUBLE_EQ(inverse->elements[index], expected.elements[index]);
    }
}

TEST(Matrix4dTests, InvertsNonUniformScaleMatrix) {
    const Math::Vector3d scale_factors{
        .x = 2.0,
        .y = 4.0,
        .z = 8.0,
    };

    const Math::Vector3d inverse_scale_factors{
        .x = 0.5,
        .y = 0.25,
        .z = 0.125,
    };

    const Math::Matrix4d matrix =
        Math::scale(scale_factors.x, scale_factors.y, scale_factors.z);

    const Math::Matrix4d expected =
        Math::scale(inverse_scale_factors.x, inverse_scale_factors.y,
                    inverse_scale_factors.z);

    const auto inverse = Math::tryInverse(matrix);

    if (!inverse.has_value()) {
        FAIL() << "Expected matrix inversion to succeed";
        return;
    }

    for (std::size_t index{}; index < expected.elements.size(); ++index) {
        EXPECT_DOUBLE_EQ(inverse->elements[index], expected.elements[index]);
    }
}

TEST(Matrix4dTests, InversionHandlesPivotRowSwapping) {
    constexpr std::size_t kFirstIndex = 0U;
    constexpr std::size_t kSecondIndex = 1U;

    constexpr double kZero = 0.0;
    constexpr double kOne = 1.0;

    Math::Matrix4d matrix = Math::matrix4dIdentity();

    matrix.at(kFirstIndex, kFirstIndex) = kZero;
    matrix.at(kSecondIndex, kSecondIndex) = kZero;
    matrix.at(kFirstIndex, kSecondIndex) = kOne;
    matrix.at(kSecondIndex, kFirstIndex) = kOne;

    const auto inverse = Math::tryInverse(matrix);

    if (!inverse.has_value()) {
        FAIL() << "Expected matrix inversion to succeed";
        return;
    }

    EXPECT_EQ(inverse->elements, matrix.elements);
}

TEST(Matrix4dTests, InvertsComposedTransformation) {
    constexpr double kRotationAngle = std::numbers::pi_v<double> / 3.0;
    constexpr double kTolerance = 1.0e-12;

    const Math::Vector3d translation{
        .x = 3.0,
        .y = -2.0,
        .z = 5.0,
    };

    const Math::Vector3d rotation_radians{
        .x = kRotationAngle,
        .y = kRotationAngle,
        .z = kRotationAngle,
    };

    const Math::Vector3d scale_factors{
        .x = 2.0,
        .y = 3.0,
        .z = 4.0,
    };

    const Math::Matrix4d matrix =
        Math::composeTRS(translation, rotation_radians, scale_factors);

    const auto inverse = Math::tryInverse(matrix);

    if (!inverse.has_value()) {
        FAIL() << "Expected matrix inversion to succeed";
        return;
    }

    const Math::Matrix4d left_product = Math::multiply(matrix, *inverse);
    const Math::Matrix4d right_product = Math::multiply(*inverse, matrix);
    const Math::Matrix4d expected = Math::matrix4dIdentity();

    for (std::size_t index{}; index < expected.elements.size(); ++index) {
        EXPECT_NEAR(left_product.elements[index], expected.elements[index],
                    kTolerance);

        EXPECT_NEAR(right_product.elements[index], expected.elements[index],
                    kTolerance);
    }
}

TEST(Matrix4dTests, InversionRejectsSingularMatrix) {
    const Math::Vector3d scale_factors{
        .x = 2.0,
        .y = 0.0,
        .z = 4.0,
    };

    const Math::Matrix4d matrix =
        Math::scale(scale_factors.x, scale_factors.y, scale_factors.z);

    const auto inverse = Math::tryInverse(matrix);

    EXPECT_FALSE(inverse.has_value());
}

TEST(Matrix4dTests, InversionRejectsNonFiniteMatrix) {
    constexpr std::size_t kFirstIndex = 0U;

    Math::Matrix4d matrix = Math::matrix4dIdentity();

    matrix.at(kFirstIndex, kFirstIndex) =
        std::numeric_limits<double>::infinity();

    EXPECT_FALSE(Math::tryInverse(matrix).has_value());

    matrix.at(kFirstIndex, kFirstIndex) =
        std::numeric_limits<double>::quiet_NaN();

    EXPECT_FALSE(Math::tryInverse(matrix).has_value());
}

TEST(Matrix4dTests, PreservesSmallOffsetsAtLargeTranslations) {
    constexpr double kLargeCoordinate = 1.0e12;
    constexpr double kSmallXOffset = 0.25;
    constexpr double kSmallYOffset = -0.5;

    const Math::Matrix4d matrix =
        Math::translate(kLargeCoordinate, -kLargeCoordinate, 0.0);

    const Math::Vector3d point{
        .x = kSmallXOffset,
        .y = kSmallYOffset,
        .z = 1.0,
    };

    const Math::Vector3d result = Math::transformPoint(matrix, point);

    EXPECT_DOUBLE_EQ(result.x, 1000000000000.25);
    EXPECT_DOUBLE_EQ(result.y, -1000000000000.5);
    EXPECT_DOUBLE_EQ(result.z, 1.0);
}

TEST(Matrix4dTests, InvertsVerySmallNonzeroScale) {
    constexpr double kSmallScale = 1.0e-12;
    constexpr double kTolerance = 1.0e-12;

    const Math::Matrix4d matrix = Math::scale(kSmallScale, 2.0, 4.0);

    const auto inverse = Math::tryInverse(matrix);

    if (!inverse.has_value()) {
        FAIL() << "Expected matrix inversion to succeed";
        return;
    }

    const Math::Matrix4d result = Math::multiply(matrix, *inverse);
    const Math::Matrix4d expected = Math::matrix4dIdentity();

    for (std::size_t index{}; index < expected.elements.size(); ++index) {
        EXPECT_NEAR(result.elements[index], expected.elements[index],
                    kTolerance);
    }
}

TEST(Matrix4dTests, ComposedTransformHandlesMixedMagnitudes) {
    const Math::Vector3d translation{
        .x = 10.0,
        .y = -4.0,
        .z = 0.0,
    };

    const Math::Vector3d rotation{
        .x = 0.0,
        .y = 0.0,
        .z = std::numbers::pi_v<double> / 2,
    };

    const Math::Vector3d scale{
        .x = 1000000.0,
        .y = 0.001,
        .z = 1.0,
    };

    const Math::Vector3d point{
        .x = 1.5,
        .y = 2.0,
        .z = 0.0,
    };

    const Math::Matrix4d matrix =
        Math::composeTRS(translation, rotation, scale);

    const Math::Vector3d result = Math::transformPoint(matrix, point);

    const Math::Vector3d expected{
        .x = 9.998,
        .y = 1499996.0,
        .z = 0.0,
    };

    constexpr double kAbsoluteTolerance = 1.0e-9;
    constexpr double kRelativeTolerance = 1.0e-12;

    EXPECT_NEAR(result.x, expected.x, kAbsoluteTolerance);
    EXPECT_NEAR(result.y, expected.y,
                kRelativeTolerance * std::abs(expected.y));
    EXPECT_DOUBLE_EQ(result.z, expected.z);
}
