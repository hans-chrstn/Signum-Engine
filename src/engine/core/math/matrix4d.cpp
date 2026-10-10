#include "matrix4d.hpp"
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
    [[nodiscard]] auto isFiniteMatrix(const Math::Matrix4d &matrix) -> bool {
        return std::ranges::all_of(matrix.elements, [](double element) -> bool {
            return std::isfinite(element);
        });
    }

    /**
     * @brief Finds the row containing the largest absolute pivot candidate.
     */
    [[nodiscard]] auto selectPivotRow(const Math::Matrix4d &matrix,
                                      std::size_t pivot_column) -> std::size_t {
        std::size_t pivot_row = pivot_column;
        double largest = std::abs(matrix.at(pivot_column, pivot_column));

        for (std::size_t candidate_row = pivot_column + 1U;
             candidate_row < Math::kMatrix4dDimension; ++candidate_row) {
            const double candidate =
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
    auto eliminatePivotColumn(Math::Matrix4d &working, Math::Matrix4d &inverse,
                              std::size_t pivot_column) -> void {
        for (std::size_t row{}; row < Math::kMatrix4dDimension; ++row) {
            if (row == pivot_column) {
                continue;
            }

            const double factor = working.at(row, pivot_column);

            for (std::size_t column{}; column < Math::kMatrix4dDimension;
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
    auto multiply(const Matrix4d &first, const Matrix4d &second) -> Matrix4d {
        Matrix4d result{};
        for (std::size_t column{}; column < kMatrix4dDimension; ++column) {
            for (std::size_t row{}; row < kMatrix4dDimension; ++row) {
                double sum = 0.0;
                for (std::size_t k{}; k < kMatrix4dDimension; ++k) {
                    sum += first.at(row, k) * second.at(k, column);
                }
                result.at(row, column) = sum;
            }
        }

        return result;
    }

    auto composeTRS(Vector3d translation, Vector3d rotation_radians,
                    Vector3d scale_factors) -> Matrix4d {
        const Matrix4d scaled =
            scale(scale_factors.x, scale_factors.y, scale_factors.z);
        const Matrix4d rotatedX = rotateX(rotation_radians.x);
        const Matrix4d rotatedY = rotateY(rotation_radians.y);
        const Matrix4d rotatedZ = rotateZ(rotation_radians.z);
        const Matrix4d translated =
            translate(translation.x, translation.y, translation.z);
        Matrix4d composed = multiply(rotatedX, scaled);
        composed = multiply(rotatedY, composed);
        composed = multiply(rotatedZ, composed);
        composed = multiply(translated, composed);

        return composed;
    }

    auto tryInverse(const Matrix4d &matrix) -> std::optional<Matrix4d> {
        if (!isFiniteMatrix(matrix)) {
            return std::nullopt;
        }

        Matrix4d working = matrix;
        Matrix4d inverse = matrix4dIdentity();

        for (std::size_t pivot_column{}; pivot_column < kMatrix4dDimension;
             ++pivot_column) {
            const std::size_t pivot_row = selectPivotRow(working, pivot_column);

            const double largest =
                std::abs(working.at(pivot_row, pivot_column));

            if (!std::isfinite(largest) || largest == 0.0) {
                return std::nullopt;
            }

            if (pivot_row != pivot_column) {
                for (std::size_t column{}; column < kMatrix4dDimension;
                     ++column) {
                    std::swap(working.at(pivot_column, column),
                              working.at(pivot_row, column));

                    std::swap(inverse.at(pivot_column, column),
                              inverse.at(pivot_row, column));
                }
            }

            const double pivot = working.at(pivot_column, pivot_column);

            for (std::size_t column{}; column < kMatrix4dDimension; ++column) {
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
