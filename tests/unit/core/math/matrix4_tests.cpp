#include "engine/core/math/matrix4.hpp"
#include <cstddef>
#include <gtest/gtest.h>
#include <numbers>

namespace Math = SNE::Engine::Core::Math;

// Compile-time tests
static_assert([] -> bool {
    constexpr auto matrix = Math::identity();

    for (std::size_t column{}; column < Math::kMatrix4Dimension; ++column) {
        for (std::size_t row{}; row < Math::kMatrix4Dimension; ++row) {
            const std::size_t index = (column * Math::kMatrix4Dimension) + row;

            const float expected = (row == column) ? 1.0F : 0.0F;

            if (matrix.elements[index] != expected) {
                return false;
            }
        }
    }

    return true;
}());

static_assert([] -> bool {
    constexpr std::size_t kTranslationRow = 1U;
    constexpr std::size_t kTranslationColumn = Math::kMatrix4Dimension - 1U;

    constexpr std::size_t kExpectedStorageIndex = 13U;
    constexpr float kExpectedTranslation = 5.0F;

    Math::Matrix4f matrix = Math::identity();

    matrix.at(kTranslationRow, kTranslationColumn) = kExpectedTranslation;

    return matrix.at(kTranslationRow, kTranslationColumn) ==
               kExpectedTranslation &&
           matrix.elements[kExpectedStorageIndex] == kExpectedTranslation;
}());

static_assert([] -> bool {
    constexpr Math::Matrix4f matrix = Math::identity();

    return matrix.at(0U, 0U) == 1.0F;
}());

static_assert(Math::translate(0.0F, 0.0F, 0.0F).elements ==
              Math::identity().elements);

static_assert([] -> bool {
    constexpr float kScaleX = 2.0F;
    constexpr float kScaleY = 3.0F;
    constexpr float kScaleZ = 4.0F;

    constexpr auto matrix = Math::scale(kScaleX, kScaleY, kScaleZ);
    constexpr std::size_t kLastIndex = Math::kMatrix4Dimension - 1U;

    return matrix.at(0U, 0U) == kScaleX && matrix.at(1U, 1U) == kScaleY &&
           matrix.at(2U, 2U) == kScaleZ &&
           matrix.at(kLastIndex, kLastIndex) == 1.0F;
}());

TEST(Matrix4Tests, IdentityMultipliedByIdentityIsIdentity) {
    const Math::Matrix4f identity = Math::identity();

    const auto result = Math::multiply(identity, identity);

    for (std::size_t i{}; i < identity.elements.size(); ++i) {
        EXPECT_FLOAT_EQ(result.elements[i], identity.elements[i]);
    }
}

TEST(Matrix4Tests, MultipliesTranslationAndScaleCorrectly) {
    // clang-format off
    const Math::Matrix4f translated{
        .elements = {
            1.0F, 0.0F, 0.0F, 0.0F,
            0.0F, 1.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 1.0F, 0.0F,
            3.0F, 2.0F, 0.0F, 1.0F,
        },
    };
    const Math::Matrix4f scaled{
        .elements = {
            2.0F, 0.0F, 0.0F, 0.0F,
            0.0F, 2.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 2.0F, 0.0F,
            0.0F, 0.0F, 0.0F, 1.0F,
        },
    };
    const Math::Matrix4f expected{
        .elements = {
            2.0F, 0.0F, 0.0F, 0.0F,
            0.0F, 2.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 2.0F, 0.0F,
            6.0F, 4.0F, 0.0F, 1.0F,
        },
    };
    // clang-format on

    // Translate first, then scale.
    const auto result = Math::multiply(scaled, translated);
    for (std::size_t i{}; i < result.elements.size(); ++i) {
        EXPECT_FLOAT_EQ(result.elements[i], expected.elements[i]);
    }
}

TEST(Matrix4Tests, MultipliesScaleThenTranslationCorrectly) {
    // clang-format off
    const Math::Matrix4f translated{
        .elements = {
            1.0F, 0.0F, 0.0F, 0.0F,
            0.0F, 1.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 1.0F, 0.0F,
            3.0F, 2.0F, 0.0F, 1.0F,
        },
    };
    const Math::Matrix4f scaled{
        .elements = {
            2.0F, 0.0F, 0.0F, 0.0F,
            0.0F, 2.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 2.0F, 0.0F,
            0.0F, 0.0F, 0.0F, 1.0F,
        },
    };
    const Math::Matrix4f expected{
        .elements = {
            2.0F, 0.0F, 0.0F, 0.0F,
            0.0F, 2.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 2.0F, 0.0F,
            3.0F, 2.0F, 0.0F, 1.0F,
        },
    };
    // clang-format on

    const auto result = Math::multiply(translated, scaled);

    for (std::size_t i{}; i < result.elements.size(); ++i) {
        EXPECT_FLOAT_EQ(result.elements[i], expected.elements[i]);
    }
}

TEST(Matrix4Tests, IdentityPreservesNonIdentityMatrix) {
    // clang-format off
    const Math::Matrix4f translated{
        .elements = {
            1.0F, 0.0F, 0.0F, 0.0F,
            0.0F, 1.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 1.0F, 0.0F,
            3.0F, 2.0F, 0.0F, 1.0F,
        },
    };
    // clang-format on

    const Math::Matrix4f identity = Math::identity();

    const auto translate_first = Math::multiply(translated, identity);
    const auto identity_first = Math::multiply(identity, translated);

    for (std::size_t i{}; i < translated.elements.size(); ++i) {
        EXPECT_FLOAT_EQ(translate_first.elements[i], translated.elements[i]);
        EXPECT_FLOAT_EQ(identity_first.elements[i], translated.elements[i]);
    }
}

TEST(Matrix4Tests, MultipliesNonUniformScaleAndTranslationCorrectly) {
    // clang-format off
    const Math::Matrix4f translated{
        .elements = {
            1.0F, 0.0F, 0.0F, 0.0F,
            0.0F, 1.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 1.0F, 0.0F,
            3.0F, 2.0F, 0.0F, 1.0F,
        },
    };
    const Math::Matrix4f non_uniform{
        .elements = {
            2.0F, 0.0F, 0.0F, 0.0F,
            0.0F, 3.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 4.0F, 0.0F,
            0.0F, 0.0F, 0.0F, 1.0F,
        },
    };
    const Math::Matrix4f expected_translate_then_scale{
        .elements = {
            2.0F, 0.0F, 0.0F, 0.0F,
            0.0F, 3.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 4.0F, 0.0F,
            6.0F, 6.0F, 0.0F, 1.0F,
        },
    };
    const Math::Matrix4f expected_scale_then_translate{
        .elements = {
            2.0F, 0.0F, 0.0F, 0.0F,
            0.0F, 3.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 4.0F, 0.0F,
            3.0F, 2.0F, 0.0F, 1.0F,
        },
    };
    // clang-format on

    const auto scale_then_translate = Math::multiply(translated, non_uniform);
    const auto translate_then_scale = Math::multiply(non_uniform, translated);

    for (std::size_t i{}; i < translated.elements.size(); ++i) {
        EXPECT_FLOAT_EQ(scale_then_translate.elements[i],
                        expected_scale_then_translate.elements[i]);

        EXPECT_FLOAT_EQ(translate_then_scale.elements[i],
                        expected_translate_then_scale.elements[i]);
    }
}

TEST(Matrix4Tests, MultipliesMatricesWithOffDiagonalValuesCorrectly) {
    // clang-format off
    const Math::Matrix4f matrix_a {
        .elements = {
            1.0F, 3.0F, 0.0F, 0.0F,
            2.0F, 4.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 1.0F, 0.0F,
            0.0F, 0.0F, 0.0F, 1.0F,
        },
    };
    const Math::Matrix4f matrix_b {
        .elements = {
            5.0F, 7.0F, 0.0F, 0.0F,
            6.0F, 8.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 1.0F, 0.0F,
            0.0F, 0.0F, 0.0F, 1.0F,
        },
    };
    const Math::Matrix4f expected {
        .elements = {
            19.0F, 43.0F, 0.0F, 0.0F,
            22.0F, 50.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 1.0F, 0.0F,
            0.0F, 0.0F, 0.0F, 1.0F,
        },
    };
    // clang-format on

    const auto result = Math::multiply(matrix_a, matrix_b);

    for (std::size_t i{}; i < result.elements.size(); ++i) {
        EXPECT_FLOAT_EQ(result.elements[i], expected.elements[i]);
    }
}

TEST(Matrix4Tests, IdentityCreatesIdentityMatrix) {
    // clang-format off
    const Math::Matrix4f expected {
            .elements = {
                1.0F, 0.0F, 0.0F, 0.0F,
                0.0F, 1.0F, 0.0F, 0.0F,
                0.0F, 0.0F, 1.0F, 0.0F,
                0.0F, 0.0F, 0.0F, 1.0F,
            },
    };
    // clang-format on

    const auto result = Math::identity();
    for (std::size_t i{}; i < result.elements.size(); ++i) {
        EXPECT_FLOAT_EQ(result.elements[i], expected.elements[i]);
    }
}

TEST(Matrix4Tests, MutableAccessorWritesCorrectElement) {
    Math::Matrix4f matrix = Math::identity();
    const float change = 5.0F;
    matrix.at(1U, 3U) = change;

    const std::size_t index = 13U;
    EXPECT_FLOAT_EQ(matrix.elements[index], change);
    const Math::Matrix4f expected = Math::identity();

    for (std::size_t i{}; i < matrix.elements.size(); ++i) {
        if (i == index) {
            continue;
        }

        EXPECT_FLOAT_EQ(matrix.elements[i], expected.elements[i]);
    }
}

TEST(Matrix4Tests, ConstAccessorReadsCorrectElement) {
    // clang-format off
    const Math::Matrix4f matrix{
        .elements = {
            19.0F, 43.0F, 0.0F, 0.0F,
            22.0F, 50.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 1.0F, 0.0F,
            0.0F, 0.0F, 0.0F, 1.0F,
        },
    };
    // clang-format on

    for (std::size_t column{}; column < Math::kMatrix4Dimension; ++column) {
        for (std::size_t row{}; row < Math::kMatrix4Dimension; ++row) {
            const std::size_t index = (column * Math::kMatrix4Dimension) + row;
            EXPECT_FLOAT_EQ(matrix.at(row, column), matrix.elements[index]);
        }
    }
}

TEST(Matrix4Tests, ConstAccessorReadsKnownCoordinates) {
    // clang-format off
    const Math::Matrix4f matrix{
        .elements = {
            19.0F, 43.0F, 0.0F, 0.0F,
            22.0F, 50.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 1.0F, 0.0F,
            0.0F, 0.0F, 0.0F, 1.0F,
        },
    };
    // clang-format on

    EXPECT_FLOAT_EQ(matrix.at(0U, 0U), 19.0F);
    EXPECT_FLOAT_EQ(matrix.at(1U, 0U), 43.0F);
    EXPECT_FLOAT_EQ(matrix.at(0U, 1U), 22.0F);
    EXPECT_FLOAT_EQ(matrix.at(1U, 1U), 50.0F);
}

TEST(Matrix4Tests, AccessorRejectsInvalidRow) {
    Math::Matrix4f matrix = Math::identity();

    EXPECT_DEATH(
        { static_cast<void>(matrix.at(Math::kMatrix4Dimension, 0U)); },
        "Matrix4f row or column is out of bounds");
}

TEST(Matrix4Tests, ConstAccessorRejectsInvalidColumn) {
    const Math::Matrix4f matrix = Math::identity();

    EXPECT_DEATH(
        { static_cast<void>(matrix.at(0U, Math::kMatrix4Dimension)); },
        "Matrix4f row or column is out of bounds");
}

TEST(Matrix4Tests, TranslateCreatesCorrectMatrix) {
    const Math::Matrix4f translated = Math::translate(3.0F, 2.0F, 5.0F);

    // clang-format off
    const Math::Matrix4f expected{
        .elements =
            {
            1.0F, 0.0F, 0.0F, 0.0F,
            0.0F, 1.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 1.0F, 0.0F,
            3.0F, 2.0F, 5.0F, 1.0F,
            },
    };
    // clang-format on

    for (std::size_t i{}; i < translated.elements.size(); ++i) {
        EXPECT_FLOAT_EQ(translated.elements[i], expected.elements[i]);
    }
}

TEST(Matrix4Tests, ScaleCreatesCorrectMatrix) {
    const auto scaled = Math::scale(2.0F, 3.0F, 4.0F);

    // clang-format off
    const Math::Matrix4f expected {
        .elements = {
            2.0F, 0.0F, 0.0F, 0.0F,
            0.0F, 3.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 4.0F, 0.0F,
            0.0F, 0.0F, 0.0F, 1.0F,
        },
    };
    // clang-format on

    for (std::size_t i{}; i < scaled.elements.size(); ++i) {
        EXPECT_FLOAT_EQ(scaled.elements[i], expected.elements[i]);
    }
}

TEST(Matrix4Tests, RotateZCreatesCorrectMatrix) {
    constexpr float kRightAngle = std::numbers::pi_v<float> / 2.0F;
    constexpr float kTolerance = 1.0e-6F;

    const auto result = Math::rotateZ(kRightAngle);

    // clang-format off
    const Math::Matrix4f expected{
        .elements = {
             0.0F, 1.0F, 0.0F, 0.0F,
            -1.0F, 0.0F, 0.0F, 0.0F,
             0.0F, 0.0F, 1.0F, 0.0F,
             0.0F, 0.0F, 0.0F, 1.0F,
        },
    };
    // clang-format on

    for (std::size_t column{}; column < Math::kMatrix4Dimension; ++column) {
        for (std::size_t row{}; row < Math::kMatrix4Dimension; ++row) {
            EXPECT_NEAR(result.at(row, column), expected.at(row, column),
                        kTolerance);
        }
    }
}

TEST(Matrix4Tests, RotateXCreatesCorrectMatrix) {
    constexpr float kRightAngle = std::numbers::pi_v<float> / 2.0F;
    constexpr float kTolerance = 1.0e-6F;

    const auto result = Math::rotateX(kRightAngle);

    // clang-format off
    const Math::Matrix4f expected{
        .elements = {
            1.0F,  0.0F, 0.0F, 0.0F,
            0.0F,  0.0F, 1.0F, 0.0F,
            0.0F, -1.0F, 0.0F, 0.0F,
            0.0F,  0.0F, 0.0F, 1.0F,
        },
    };
    // clang-format on

    for (std::size_t column{}; column < Math::kMatrix4Dimension; ++column) {
        for (std::size_t row{}; row < Math::kMatrix4Dimension; ++row) {
            EXPECT_NEAR(result.at(row, column), expected.at(row, column),
                        kTolerance);
        }
    }
}

TEST(Matrix4Tests, RotateYCreatesCorrectMatrix) {
    constexpr float kRightAngle = std::numbers::pi_v<float> / 2.0F;
    constexpr float kTolerance = 1.0e-6F;

    const auto result = Math::rotateY(kRightAngle);

    // clang-format off
    const Math::Matrix4f expected{
        .elements = {
             0.0F, 0.0F, -1.0F, 0.0F,
             0.0F, 1.0F,  0.0F, 0.0F,
             1.0F, 0.0F,  0.0F, 0.0F,
             0.0F, 0.0F,  0.0F, 1.0F,
        },
    };
    // clang-format on

    for (std::size_t column{}; column < Math::kMatrix4Dimension; ++column) {
        for (std::size_t row{}; row < Math::kMatrix4Dimension; ++row) {
            EXPECT_NEAR(result.at(row, column), expected.at(row, column),
                        kTolerance);
        }
    }
}

TEST(Matrix4Tests, TransformPointAppliesTranslation) {
    const Math::Vector3f point{
        .x = 1.0F,
        .y = 2.0F,
        .z = 3.0F,
    };

    const Math::Matrix4f matrix = Math::translate(3.0F, 2.0F, 5.0F);

    const Math::Vector3f expected{
        .x = 4.0F,
        .y = 4.0F,
        .z = 8.0F,
    };

    const auto result = Math::transformPoint(matrix, point);

    EXPECT_FLOAT_EQ(result.x, expected.x);
    EXPECT_FLOAT_EQ(result.y, expected.y);
    EXPECT_FLOAT_EQ(result.z, expected.z);
}

TEST(Matrix4Tests, TransformDirectionIgnoresTranslation) {
    const Math::Vector3f direction{
        .x = 1.0F,
        .y = 2.0F,
        .z = 3.0F,
    };

    const Math::Matrix4f matrix = Math::translate(3.0F, 2.0F, 5.0F);

    const Math::Vector3f expected{
        .x = 1.0F,
        .y = 2.0F,
        .z = 3.0F,
    };

    const auto result = Math::transformDirection(matrix, direction);

    EXPECT_FLOAT_EQ(result.x, expected.x);
    EXPECT_FLOAT_EQ(result.y, expected.y);
    EXPECT_FLOAT_EQ(result.z, expected.z);
}

TEST(Matrix4Tests, TransformPointAppliesRotation) {
    constexpr float kRightAngle = std::numbers::pi_v<float> / 2.0F;
    constexpr float kTolerance = 1.0e-6F;

    const Math::Vector3f point{
        .x = 1.0F,
        .y = 0.0F,
        .z = 0.0F,
    };

    const Math::Matrix4f matrix = Math::rotateZ(kRightAngle);

    const Math::Vector3f expected{
        .x = 0.0F,
        .y = 1.0F,
        .z = 0.0F,
    };

    const auto result = Math::transformPoint(matrix, point);

    EXPECT_NEAR(result.x, expected.x, kTolerance);
    EXPECT_NEAR(result.y, expected.y, kTolerance);
    EXPECT_NEAR(result.z, expected.z, kTolerance);
}

TEST(Matrix4Tests, TransformDirectionAppliesRotation) {
    constexpr float kRightAngle = std::numbers::pi_v<float> / 2.0F;
    constexpr float kTolerance = 1.0e-6F;

    const Math::Vector3f direction{
        .x = 1.0F,
        .y = 0.0F,
        .z = 0.0F,
    };

    const Math::Matrix4f matrix = Math::rotateZ(kRightAngle);

    const Math::Vector3f expected{
        .x = 0.0F,
        .y = 1.0F,
        .z = 0.0F,
    };

    const auto result = Math::transformDirection(matrix, direction);

    EXPECT_NEAR(result.x, expected.x, kTolerance);
    EXPECT_NEAR(result.y, expected.y, kTolerance);
    EXPECT_NEAR(result.z, expected.z, kTolerance);
}

TEST(Matrix4Tests, TransformPointAppliesNonUniformScale) {
    const Math::Matrix4f matrix = Math::scale(2.0F, 3.0F, 4.0F);

    const Math::Vector3f point{
        .x = 1.0F,
        .y = 2.0F,
        .z = 3.0F,
    };

    const Math::Vector3f expected{
        .x = 2.0F,
        .y = 6.0F,
        .z = 12.0F,
    };

    const auto result = Math::transformPoint(matrix, point);

    EXPECT_FLOAT_EQ(result.x, expected.x);
    EXPECT_FLOAT_EQ(result.y, expected.y);
    EXPECT_FLOAT_EQ(result.z, expected.z);
}

TEST(Matrix4Tests, TransformDirectionAppliesNonUniformScale) {
    const Math::Matrix4f matrix = Math::scale(2.0F, 3.0F, 4.0F);

    const Math::Vector3f direction{
        .x = 1.0F,
        .y = 2.0F,
        .z = 3.0F,
    };

    const Math::Vector3f expected{
        .x = 2.0F,
        .y = 6.0F,
        .z = 12.0F,
    };

    const auto result = Math::transformDirection(matrix, direction);

    EXPECT_FLOAT_EQ(result.x, expected.x);
    EXPECT_FLOAT_EQ(result.y, expected.y);
    EXPECT_FLOAT_EQ(result.z, expected.z);
}

TEST(Matrix4Tests, TransposeSwapsRowsAndColumns) {
    constexpr std::size_t kFirstIndex = 0U;
    constexpr std::size_t kSecondIndex = 1U;
    constexpr std::size_t kThirdIndex = 2U;
    constexpr std::size_t kFourthIndex = 3U;

    constexpr float kUpperValue = 2.0F;
    constexpr float kLowerValue = 3.0F;
    constexpr float kTranslationValue = 7.0F;

    Math::Matrix4f matrix = Math::identity();

    matrix.at(kFirstIndex, kSecondIndex) = kUpperValue;
    matrix.at(kSecondIndex, kFirstIndex) = kLowerValue;
    matrix.at(kThirdIndex, kFourthIndex) = kTranslationValue;

    const Math::Matrix4f result = Math::transpose(matrix);

    EXPECT_FLOAT_EQ(result.at(kSecondIndex, kFirstIndex), kUpperValue);
    EXPECT_FLOAT_EQ(result.at(kFirstIndex, kSecondIndex), kLowerValue);
    EXPECT_FLOAT_EQ(result.at(kFourthIndex, kThirdIndex), kTranslationValue);
}

TEST(Matrix4Tests, TransposingTwiceRestoresOriginalMatrix) {
    constexpr std::size_t kFirstIndex = 0U;
    constexpr std::size_t kSecondIndex = 1U;
    constexpr std::size_t kThirdIndex = 2U;

    constexpr float kFirstValue = 5.0F;
    constexpr float kSecondValue = -3.0F;

    Math::Matrix4f matrix = Math::identity();

    matrix.at(kFirstIndex, kSecondIndex) = kFirstValue;
    matrix.at(kSecondIndex, kThirdIndex) = kSecondValue;

    const Math::Matrix4f transposed = Math::transpose(matrix);
    const Math::Matrix4f restored = Math::transpose(transposed);

    EXPECT_EQ(restored.elements, matrix.elements);
}

TEST(Matrix4Tests, ComposeTRSAppliesScaleRotationAndTranslation) {
    constexpr float kRightAngle = std::numbers::pi_v<float> / 2.0F;
    constexpr float kTolerance = 1.0e-5F;

    const Math::Vector3f translation{
        .x = 5.0F,
        .y = -2.0F,
        .z = 3.0F,
    };

    const Math::Vector3f rotation_radians{
        .x = 0.0F,
        .y = 0.0F,
        .z = kRightAngle,
    };

    const Math::Vector3f scale_factors{
        .x = 2.0F,
        .y = 3.0F,
        .z = 4.0F,
    };

    const Math::Vector3f point{
        .x = 1.0F,
        .y = 2.0F,
        .z = -1.0F,
    };

    const Math::Vector3f expected{
        .x = -1.0F,
        .y = 0.0F,
        .z = -1.0F,
    };

    const Math::Matrix4f matrix =
        Math::composeTRS(translation, rotation_radians, scale_factors);

    const Math::Vector3f result = Math::transformPoint(matrix, point);

    EXPECT_NEAR(result.x, expected.x, kTolerance);
    EXPECT_NEAR(result.y, expected.y, kTolerance);
    EXPECT_NEAR(result.z, expected.z, kTolerance);
}

TEST(Matrix4Tests, ComposeTRSWithIdentityRotationAndScaleMatchesTranslation) {
    constexpr float kTolerance = 1.0e-5F;

    const Math::Vector3f translation{
        .x = 3.0F,
        .y = -4.0F,
        .z = 5.0F,
    };

    const Math::Vector3f rotation_radians{};

    const Math::Vector3f scale_factors{
        .x = 1.0F,
        .y = 1.0F,
        .z = 1.0F,
    };

    const Math::Matrix4f result =
        Math::composeTRS(translation, rotation_radians, scale_factors);

    const Math::Matrix4f expected =
        Math::translate(translation.x, translation.y, translation.z);

    for (std::size_t index{}; index < expected.elements.size(); ++index) {
        EXPECT_NEAR(result.elements[index], expected.elements[index],
                    kTolerance);
    }
}

TEST(Matrix4Tests, InvertsIdentityMatrix) {
    const Math::Matrix4f matrix = Math::identity();

    const auto inverse = Math::tryInverse(matrix);

    if (!inverse.has_value()) {
        ADD_FAILURE() << "Expected matrix inversion to succeed";
        return;
    }

    EXPECT_EQ(inverse->elements, matrix.elements);
}

TEST(Matrix4Tests, InvertsTranslationMatrix) {
    constexpr float kTolerance = 1.0e-5F;

    const Math::Vector3f translation{
        .x = 3.0F,
        .y = -4.0F,
        .z = 5.0F,
    };

    const Math::Vector3f inverse_translation{
        .x = -3.0F,
        .y = 4.0F,
        .z = -5.0F,
    };

    const Math::Matrix4f matrix =
        Math::translate(translation.x, translation.y, translation.z);

    const Math::Matrix4f expected = Math::translate(
        inverse_translation.x, inverse_translation.y, inverse_translation.z);

    const auto inverse = Math::tryInverse(matrix);

    if (!inverse.has_value()) {
        ADD_FAILURE() << "Expected matrix inversion to succeed";
        return;
    }

    for (std::size_t index{}; index < expected.elements.size(); ++index) {
        EXPECT_NEAR(inverse->elements[index], expected.elements[index],
                    kTolerance);
    }
}

TEST(Matrix4Tests, InvertsNonUniformScaleMatrix) {
    constexpr float kTolerance = 1.0e-5F;

    const Math::Vector3f scale_factors{
        .x = 2.0F,
        .y = 4.0F,
        .z = 8.0F,
    };

    const Math::Vector3f inverse_scale_factors{
        .x = 0.5F,
        .y = 0.25F,
        .z = 0.125F,
    };

    const Math::Matrix4f matrix =
        Math::scale(scale_factors.x, scale_factors.y, scale_factors.z);

    const Math::Matrix4f expected =
        Math::scale(inverse_scale_factors.x, inverse_scale_factors.y,
                    inverse_scale_factors.z);

    const auto inverse = Math::tryInverse(matrix);

    if (!inverse.has_value()) {
        ADD_FAILURE() << "Expected matrix inversion to succeed";
        return;
    }

    for (std::size_t index{}; index < expected.elements.size(); ++index) {
        EXPECT_NEAR(inverse->elements[index], expected.elements[index],
                    kTolerance);
    }
}

TEST(Matrix4Tests, InversionHandlesPivotRowSwapping) {
    constexpr std::size_t kFirstIndex = 0U;
    constexpr std::size_t kSecondIndex = 1U;

    constexpr float kZero = 0.0F;
    constexpr float kOne = 1.0F;

    Math::Matrix4f matrix = Math::identity();

    matrix.at(kFirstIndex, kFirstIndex) = kZero;
    matrix.at(kSecondIndex, kSecondIndex) = kZero;
    matrix.at(kFirstIndex, kSecondIndex) = kOne;
    matrix.at(kSecondIndex, kFirstIndex) = kOne;

    const auto inverse = Math::tryInverse(matrix);

    if (!inverse.has_value()) {
        ADD_FAILURE() << "Expected matrix inversion to succeed";
        return;
    }

    EXPECT_EQ(inverse->elements, matrix.elements);
}

TEST(Matrix4Tests, InvertsComposedTransformation) {
    constexpr float kRotationAngle = std::numbers::pi_v<float> / 3.0F;
    constexpr float kTolerance = 1.0e-4F;

    const Math::Vector3f translation{
        .x = 3.0F,
        .y = -2.0F,
        .z = 5.0F,
    };

    const Math::Vector3f rotation_radians{
        .x = kRotationAngle,
        .y = kRotationAngle,
        .z = kRotationAngle,
    };

    const Math::Vector3f scale_factors{
        .x = 2.0F,
        .y = 3.0F,
        .z = 4.0F,
    };

    const Math::Matrix4f matrix =
        Math::composeTRS(translation, rotation_radians, scale_factors);

    const auto inverse = Math::tryInverse(matrix);

    if (!inverse.has_value()) {
        ADD_FAILURE() << "Expected matrix inversion to succeed";
        return;
    }

    const Math::Matrix4f left_product = Math::multiply(matrix, *inverse);

    const Math::Matrix4f right_product = Math::multiply(*inverse, matrix);

    const Math::Matrix4f expected = Math::identity();

    for (std::size_t index{}; index < expected.elements.size(); ++index) {
        EXPECT_NEAR(left_product.elements[index], expected.elements[index],
                    kTolerance);

        EXPECT_NEAR(right_product.elements[index], expected.elements[index],
                    kTolerance);
    }
}

TEST(Matrix4Tests, InversionRejectsSingularMatrix) {
    const Math::Vector3f scale_factors{
        .x = 2.0F,
        .y = 0.0F,
        .z = 4.0F,
    };

    const Math::Matrix4f matrix =
        Math::scale(scale_factors.x, scale_factors.y, scale_factors.z);

    const auto inverse = Math::tryInverse(matrix);

    EXPECT_FALSE(inverse.has_value());
}

TEST(Matrix4Tests, InversionRejectsNonFiniteMatrix) {
    constexpr std::size_t kFirstIndex = 0U;

    Math::Matrix4f matrix = Math::identity();

    matrix.at(kFirstIndex, kFirstIndex) =
        std::numeric_limits<float>::infinity();

    EXPECT_FALSE(Math::tryInverse(matrix).has_value());

    matrix.at(kFirstIndex, kFirstIndex) =
        std::numeric_limits<float>::quiet_NaN();

    EXPECT_FALSE(Math::tryInverse(matrix).has_value());
}
