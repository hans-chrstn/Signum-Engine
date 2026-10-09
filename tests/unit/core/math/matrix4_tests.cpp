#include "engine/core/math/matrix4.hpp"
#include <cstddef>
#include <gtest/gtest.h>

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
