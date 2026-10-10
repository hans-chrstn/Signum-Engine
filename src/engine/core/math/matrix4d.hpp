#pragma once

#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/core/math/vector3d.hpp"
#include <array>
#include <cmath>
#include <cstddef>
#include <optional>

namespace SNE::Engine::Core::Math {
    /**
     * @brief Number of rows and columns in a 4x4 matrix.
     */
    constexpr std::size_t kMatrix4dDimension = 4U;

    /**
     * @brief Represents a 4x4 double-precision floating-point matrix.
     *
     * Stores sixteen double-precision elements in column-major order.
     *
     * Mathematical element access uses row and column coordinates
     * independently of the underlying storage layout.
     *
     * The element at a given row and column is stored at:
     * column * kMatrix4dDimension + row.
     *
     * Default initialization produces a zero matrix, not an identity matrix.
     *
     * The type follows the engine's column-vector transformation
     * convention and is independent of rendering APIs.
     *
     * No SIMD-specific alignment or GPU storage layout is imposed.
     */
    struct Matrix4d {
        /**
         * @brief Matrix elements stored in column-major order.
         *
         * Contains sixteen double-precision values initialized to zero
         * by default.
         */
        std::array<double, kMatrix4dDimension * kMatrix4dDimension> elements{};

        /**
         * @brief Provides mutable access to a matrix element.
         *
         * Accesses an element using mathematical row and column coordinates
         * while internally preserving column-major storage.
         *
         * @param row Zero-based row index.
         * @param column Zero-based column index.
         *
         * @pre Both indices must be less than kMatrix4dDimension.
         *
         * @return A mutable reference to the requested matrix element.
         */
        [[nodiscard]] constexpr auto at(std::size_t row, std::size_t column)
            -> double & {
            if (row >= kMatrix4dDimension || column >= kMatrix4dDimension) {
                Assertion::failAssertion(
                    Assertion::AssertionType::Precondition,
                    Error::Subsystem::Core,
                    "Matrix4d row or column is out of bounds");
            }
            const std::size_t index = (column * kMatrix4dDimension) + row;
            return elements[index];
        }

        /**
         * @brief Provides read-only access to a matrix element.
         *
         * Accesses an element using mathematical row and column coordinates
         * without modifying the matrix.
         *
         * @param row Zero-based row index.
         * @param column Zero-based column index.
         *
         * @pre Both indices must be less than kMatrix4dDimension.
         *
         * @return A const reference to the requested matrix element.
         */
        [[nodiscard]] constexpr auto at(std::size_t row,
                                        std::size_t column) const -> const
            double & {
            if (row >= kMatrix4dDimension || column >= kMatrix4dDimension) {
                Assertion::failAssertion(
                    Assertion::AssertionType::Precondition,
                    Error::Subsystem::Core,
                    "Matrix4d row or column is out of bounds");
            }
            const std::size_t index = (column * kMatrix4dDimension) + row;
            return elements[index];
        }
    };

    /**
     * @brief Multiplies two 4x4 matrices.
     *
     * Computes the matrix product first * second using standard
     * row-by-column multiplication.
     *
     * Both input matrices and the result use column-major storage.
     * The input matrices are not modified.
     *
     * Matrix multiplication is order-dependent; swapping the operands
     * can produce a different result.
     *
     * @param first Left-hand matrix operand.
     * @param second Right-hand matrix operand.
     *
     * @return A new Matrix4d containing the product first * second.
     */
    [[nodiscard]] auto multiply(const Matrix4d &first, const Matrix4d &second)
        -> Matrix4d;

    /**
     * @brief Creates a 4x4 identity matrix.
     *
     * @return A matrix with ones on the main diagonal
     *         and zeros everywhere else.
     */
    [[nodiscard]] constexpr auto matrix4dIdentity() -> Matrix4d {
        return Matrix4d{
            // clang-format off
            .elements = {
                1.0, 0.0, 0.0, 0.0,
                0.0, 1.0, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                0.0, 0.0, 0.0, 1.0,
            },
            // clang-format on
        };
    }

    /**
     * @brief Creates a 4x4 translation matrix.
     *
     * Constructs a matrix that translates positions along the X, Y,
     * and Z axes using the provided offsets.
     *
     * The matrix follows the column-vector transformation convention
     * and stores its translation components in the final column
     * using column-major storage.
     *
     * The resulting matrix contains an identity matrix with the
     * translation components replacing the first three elements
     * of its final column.
     *
     * @param translation_x Translation offset along the X axis.
     * @param translation_y Translation offset along the Y axis.
     * @param translation_z Translation offset along the Z axis.
     *
     * @return A new Matrix4d representing the specified translation.
     */
    [[nodiscard]] constexpr auto translate(double translation_x,
                                           double translation_y,
                                           double translation_z) -> Matrix4d {
        Matrix4d result = matrix4dIdentity();
        constexpr std::size_t kTranslationColumn = kMatrix4dDimension - 1U;
        result.at(0U, kTranslationColumn) = translation_x;
        result.at(1U, kTranslationColumn) = translation_y;
        result.at(2U, kTranslationColumn) = translation_z;

        return result;
    }

    /**
     * @brief Creates a 4x4 nonuniform scaling matrix.
     *
     * Constructs a matrix that scales positions along the X, Y,
     * and Z axes using the provided factors.
     *
     * The matrix follows the column-vector transformation convention
     * and stores its scaling components along the main diagonal
     * using column-major storage.
     *
     * A scaling factor of 1 preserves the corresponding axis.
     * A factor of 0 collapses that axis, while a negative factor
     * reverses its direction.
     *
     * @param scale_x Scaling factor along the X axis.
     * @param scale_y Scaling factor along the Y axis.
     * @param scale_z Scaling factor along the Z axis.
     *
     * @return A new Matrix4d representing the specified scaling.
     */
    [[nodiscard]] constexpr auto scale(double scale_x, double scale_y,
                                       double scale_z) -> Matrix4d {
        Matrix4d result = matrix4dIdentity();
        result.at(0U, 0U) = scale_x;
        result.at(1U, 1U) = scale_y;
        result.at(2U, 2U) = scale_z;

        return result;
    }

    /**
     * @brief Creates a 4x4 rotation matrix around the Z axis.
     *
     * Constructs a matrix that rotates positions and directions
     * around the Z axis using the specified angle in radians.
     *
     * The matrix follows the right-handed coordinate system
     * and column-vector transformation convention.
     *
     * Positive angles produce counterclockwise rotation
     * in the XY plane, rotating the positive X axis toward
     * the positive Y axis.
     *
     * The Z coordinate and homogeneous W component remain
     * unchanged by the resulting transformation.
     *
     * @param radians Rotation angle in radians.
     *
     * @return A new Matrix4d representing the Z-axis rotation.
     */
    [[nodiscard]] inline auto rotateZ(double radians) -> Matrix4d {
        Matrix4d result = matrix4dIdentity();

        const double cosine = std::cos(radians);
        const double sine = std::sin(radians);

        result.at(0U, 0U) = cosine;
        result.at(1U, 0U) = sine;
        result.at(0U, 1U) = -sine;
        result.at(1U, 1U) = cosine;

        return result;
    }

    /**
     * @brief Creates a 4x4 rotation matrix around the X axis.
     *
     * Constructs a matrix that rotates positions and directions
     * around the X axis using the specified angle in radians.
     *
     * The matrix follows the right-handed coordinate system
     * and column-vector transformation convention.
     *
     * Positive angles produce counterclockwise rotation
     * in the YZ plane when viewed along the positive X axis
     * toward the origin, rotating the positive Y axis toward
     * the positive Z axis.
     *
     * The X coordinate and homogeneous W component remain
     * unchanged by the resulting transformation.
     *
     * @param radians Rotation angle in radians.
     *
     * @return A new Matrix4d representing the X-axis rotation.
     */
    [[nodiscard]] inline auto rotateX(double radians) -> Matrix4d {
        Matrix4d result = matrix4dIdentity();

        const double cosine = std::cos(radians);
        const double sine = std::sin(radians);

        result.at(1U, 1U) = cosine;
        result.at(2U, 1U) = sine;
        result.at(1U, 2U) = -sine;
        result.at(2U, 2U) = cosine;

        return result;
    }

    /**
     * @brief Creates a 4x4 rotation matrix around the Y axis.
     *
     * Constructs a matrix that rotates positions and directions
     * around the Y axis using the specified angle in radians.
     *
     * The matrix follows the right-handed coordinate system
     * and column-vector transformation convention.
     *
     * Positive angles produce counterclockwise rotation
     * in the ZX plane when viewed along the positive Y axis
     * toward the origin, rotating the positive Z axis toward
     * the positive X axis.
     *
     * The Y coordinate and homogeneous W component remain
     * unchanged by the resulting transformation.
     *
     * @param radians Rotation angle in radians.
     *
     * @return A new Matrix4d representing the Y-axis rotation.
     */
    [[nodiscard]] inline auto rotateY(double radians) -> Matrix4d {
        Matrix4d result = matrix4dIdentity();

        const double cosine = std::cos(radians);
        const double sine = std::sin(radians);

        result.at(0U, 0U) = cosine;
        result.at(0U, 2U) = sine;
        result.at(2U, 0U) = -sine;
        result.at(2U, 2U) = cosine;

        return result;
    }

    /**
     * @brief Transforms a three-dimensional direction using a 4x4 matrix.
     *
     * Applies the linear portion of an affine transformation matrix
     * to the provided direction vector.
     *
     * The transformation follows the column-vector convention
     * and uses the upper-left 3x3 portion of the matrix.
     *
     * The direction is treated as having a homogeneous W component
     * of zero, so translation does not affect the result.
     *
     * Rotation and scaling affect the direction. Nonuniform scaling
     * may change its magnitude, and the result is not normalized.
     *
     * The input matrix and direction are not modified.
     *
     * @param matrix Affine transformation matrix to apply.
     * @param direction Direction vector to transform.
     *
     * @pre The matrix must represent an affine 3D transformation
     *      with a final row of [0, 0, 0, 1].
     *
     * @return A new Vector3d containing the transformed direction.
     */
    [[nodiscard]] constexpr auto transformDirection(const Matrix4d &matrix,
                                                    Vector3d direction)
        -> Vector3d {
        return Vector3d{
            .x = (matrix.at(0U, 0U) * direction.x) +
                 (matrix.at(0U, 1U) * direction.y) +
                 (matrix.at(0U, 2U) * direction.z),
            .y = (matrix.at(1U, 0U) * direction.x) +
                 (matrix.at(1U, 1U) * direction.y) +
                 (matrix.at(1U, 2U) * direction.z),
            .z = (matrix.at(2U, 0U) * direction.x) +
                 (matrix.at(2U, 1U) * direction.y) +
                 (matrix.at(2U, 2U) * direction.z),
        };
    }

    /**
     * @brief Transforms a three-dimensional point using a 4x4 matrix.
     *
     * Applies an affine transformation matrix to the provided
     * position vector.
     *
     * The transformation follows the column-vector convention
     * and uses the upper-left 3x3 portion of the matrix
     * together with the translation components in its final column.
     *
     * The point is treated as having a homogeneous W component
     * of one, allowing translation, rotation, and scaling
     * to affect its position.
     *
     * The function returns Cartesian coordinates without performing
     * perspective division and does not support general projective
     * transformations.
     *
     * The input matrix and point are not modified.
     *
     * @param matrix Affine transformation matrix to apply.
     * @param point Position vector to transform.
     *
     * @pre The matrix must represent an affine 3D transformation
     *      with a final row of [0, 0, 0, 1].
     *
     * @return A new Vector3d containing the transformed position.
     */
    [[nodiscard]] constexpr auto transformPoint(const Matrix4d &matrix,
                                                Vector3d point) -> Vector3d {
        return Vector3d{
            .x = (matrix.at(0U, 0U) * point.x) + (matrix.at(0U, 1U) * point.y) +
                 (matrix.at(0U, 2U) * point.z) + matrix.at(0U, 3U),
            .y = (matrix.at(1U, 0U) * point.x) + (matrix.at(1U, 1U) * point.y) +
                 (matrix.at(1U, 2U) * point.z) + matrix.at(1U, 3U),
            .z = (matrix.at(2U, 0U) * point.x) + (matrix.at(2U, 1U) * point.y) +
                 (matrix.at(2U, 2U) * point.z) + matrix.at(2U, 3U),
        };
    }

    /**
     * @brief Composes a 3D affine transformation from translation, rotation,
     * and scale.
     *
     * Uses column vectors and applies nonuniform scaling first, followed
     * by X, Y, and Z rotations, then translation.
     *
     * The resulting matrix is T * Rz * Ry * Rx * S.
     *
     * @param translation Translation along the X, Y, and Z axes.
     * @param rotation_radians Rotation angles around the X, Y, and Z axes,
     *                         expressed in radians.
     * @param scale_factors Scaling factors along the X, Y, and Z axes.
     *
     * @return The composed affine transformation matrix.
     */
    [[nodiscard]] auto composeTRS(Vector3d translation,
                                  Vector3d rotation_radians,
                                  Vector3d scale_factors) -> Matrix4d;

    /**
     * @brief Computes the transpose of a 4x4 matrix.
     *
     * Exchanges rows and columns without changing the matrix storage
     * convention.
     *
     * @param matrix Matrix to transpose.
     *
     * @return A new matrix containing the transposed elements.
     */
    [[nodiscard]] constexpr auto transpose(const Matrix4d &matrix) -> Matrix4d {
        Matrix4d result{};
        for (std::size_t column{}; column < kMatrix4dDimension; ++column) {
            for (std::size_t row{}; row < kMatrix4dDimension; ++row) {
                // clang-format off
                result.at(row, column) = matrix.at(column, row); // NOLINT(readability-suspicious-call-argument)
                // clang-format on
            }
        }

        return result;
    }

    /**
     * @brief Attempts to invert a 4x4 matrix.
     *
     * Uses Gauss-Jordan elimination with partial pivoting.
     *
     * Rejects non-finite input matrices and matrices for which
     * elimination cannot produce a finite inverse.
     *
     * Near-singular matrices may produce numerically inaccurate results;
     * no condition-number threshold is currently applied.
     *
     * @param matrix Matrix to invert.
     *
     * @return The inverse matrix when successful, or std::nullopt
     *         when inversion fails.
     */
    [[nodiscard]] auto tryInverse(const Matrix4d &matrix)
        -> std::optional<Matrix4d>;

} // namespace SNE::Engine::Core::Math
