#include "matrix4.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <optional>
#include <utility>

namespace Math = SNE::Engine::Core::Math;

namespace {
    /**
     * @brief Checks whether all matrix elements are finite.
     */
    [[nodiscard]] auto isFiniteMatrix(const Math::Matrix4f &matrix) -> bool {
        return std::ranges::all_of(matrix.elements, [](float element) -> bool {
            return std::isfinite(element);
        });
    }

    /**
     * @brief Finds the row containing the largest absolute pivot candidate.
     */
    [[nodiscard]] auto selectPivotRow(const Math::Matrix4f &matrix,
                                      std::size_t pivot_column) -> std::size_t {
        std::size_t pivot_row = pivot_column;
        float largest = std::abs(matrix.at(pivot_column, pivot_column));

        for (std::size_t candidate_row = pivot_column + 1U;
             candidate_row < Math::kMatrix4Dimension; ++candidate_row) {
            const float candidate =
                std::abs(matrix.at(candidate_row, pivot_column));

            if (candidate > largest) {
                largest = candidate;
                pivot_row = candidate_row;
            }
        }

        return pivot_row;
    }

    /**
     * @brief Eliminates non-pivot entries in the current pivot column.
     */
    auto eliminatePivotColumn(Math::Matrix4f &working, Math::Matrix4f &inverse,
                              std::size_t pivot_column) -> void {
        for (std::size_t row{}; row < Math::kMatrix4Dimension; ++row) {
            if (row == pivot_column) {
                continue;
            }

            const float factor = working.at(row, pivot_column);

            for (std::size_t column{}; column < Math::kMatrix4Dimension;
                 ++column) {
                working.at(row, column) -=
                    factor * working.at(pivot_column, column);

                inverse.at(row, column) -=
                    factor * inverse.at(pivot_column, column);
            }
        }
    }
} // namespace

namespace SNE::Engine::Core::Math {
    auto multiply(const Matrix4f &first, const Matrix4f &second) -> Matrix4f {
        Matrix4f result{};
        for (std::size_t column{}; column < kMatrix4Dimension; ++column) {
            for (std::size_t row{}; row < kMatrix4Dimension; ++row) {
                float sum = 0.0F;
                for (std::size_t k{}; k < kMatrix4Dimension; ++k) {
                    sum += first.at(row, k) * second.at(k, column);
                }
                result.at(row, column) = sum;
            }
        }

        return result;
    }

    auto composeTRS(Vector3f translation, Vector3f rotation_radians,
                    Vector3f scale_factors) -> Matrix4f {
        Matrix4f scaled =
            scale(scale_factors.x, scale_factors.y, scale_factors.z);
        Matrix4f rotatedX = rotateX(rotation_radians.x);
        Matrix4f rotatedY = rotateY(rotation_radians.y);
        Matrix4f rotatedZ = rotateZ(rotation_radians.z);
        Matrix4f translated =
            translate(translation.x, translation.y, translation.z);
        Matrix4f composed = multiply(rotatedX, scaled);
        composed = multiply(rotatedY, composed);
        composed = multiply(rotatedZ, composed);
        composed = multiply(translated, composed);

        return composed;
    }

    auto tryInverse(const Matrix4f &matrix) -> std::optional<Matrix4f> {
        if (!isFiniteMatrix(matrix)) {
            return std::nullopt;
        }

        Matrix4f working = matrix;
        Matrix4f inverse = identity();

        for (std::size_t pivot_column{}; pivot_column < kMatrix4Dimension;
             ++pivot_column) {
            const std::size_t pivot_row = selectPivotRow(working, pivot_column);

            const float largest = std::abs(working.at(pivot_row, pivot_column));

            if (largest == 0.0F || !std::isfinite(largest)) {
                return std::nullopt;
            }

            if (pivot_row != pivot_column) {
                for (std::size_t column{}; column < kMatrix4Dimension;
                     ++column) {
                    std::swap(working.at(pivot_column, column),
                              working.at(pivot_row, column));

                    std::swap(inverse.at(pivot_column, column),
                              inverse.at(pivot_row, column));
                }
            }

            const float pivot = working.at(pivot_column, pivot_column);

            for (std::size_t column{}; column < kMatrix4Dimension; ++column) {
                working.at(pivot_column, column) /= pivot;
                inverse.at(pivot_column, column) /= pivot;
            }

            eliminatePivotColumn(working, inverse, pivot_column);
        }

        if (!isFiniteMatrix(working) || !isFiniteMatrix(inverse)) {
            return std::nullopt;
        }

        return inverse;
    }

} // namespace SNE::Engine::Core::Math
