#include "matrix4.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numbers>
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

    auto perspectiveRH_ZO(PerspectiveProjectionParameters parameters)
        -> Matrix4f {
        const auto fail_invalid_parameters = [] -> void {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition, Error::Subsystem::Core,
                "Perspective projection parameters are invalid");
        };

        if (!std::isfinite(parameters.fov_y_radians) ||
            !std::isfinite(parameters.aspect_ratio) ||
            !std::isfinite(parameters.near_plane) ||
            !std::isfinite(parameters.far_plane)) {
            fail_invalid_parameters();
        }

        if (parameters.fov_y_radians <= 0.0F ||
            parameters.fov_y_radians >= std::numbers::pi_v<float>) {
            fail_invalid_parameters();
        }

        if (parameters.aspect_ratio <= 0.0F) {
            fail_invalid_parameters();
        }

        if (parameters.near_plane <= 0.0F) {
            fail_invalid_parameters();
        }

        if (parameters.far_plane <= parameters.near_plane) {
            fail_invalid_parameters();
        }

        const float focal_scale =
            1.0F / std::tan(parameters.fov_y_radians / 2.0F);

        Matrix4f matrix{};
        matrix.at(0U, 0U) = focal_scale / parameters.aspect_ratio;
        matrix.at(1U, 1U) = focal_scale;
        matrix.at(2U, 2U) = parameters.far_plane /
                            (parameters.near_plane - parameters.far_plane);
        matrix.at(2U, 3U) = (parameters.far_plane * parameters.near_plane) /
                            (parameters.near_plane - parameters.far_plane);
        matrix.at(3U, 2U) = -1.0F;
        return matrix;
    }
} // namespace SNE::Engine::Core::Math
