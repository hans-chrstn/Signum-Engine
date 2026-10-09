#pragma once

#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/core/math/vector3.hpp"
#include <array>
#include <cmath>
#include <cstddef>

namespace SNE::Engine::Core::Math {
    /**
     * @brief Number of rows and columns in a 4x4 matrix.
     */
    constexpr std::size_t kMatrix4Dimension = 4U;

    /**
     * @brief Represents a 4x4 single-precision floating-point matrix.
     *
     * Stores sixteen elements in column-major order.
     *
     * The element at a given row and column is accessed using:
     * column * kMatrix4Dimension + row.
     *
     * Default initialization produces a zero matrix, not an identity matrix.
     *
     * This type is independent of rendering APIs.
     */
    struct Matrix4f {
        /**
         * @brief Matrix elements stored in column-major order.
         *
         * Contains sixteen floating-point values initialized to zero
         * by default.
         */
        std::array<float, kMatrix4Dimension * kMatrix4Dimension> elements{};

        /**
         * @brief Provides mutable access to a matrix element.
         *
         * Accesses an element using mathematical row and column coordinates
         * while internally preserving column-major storage.
         *
         * @param row Zero-based row index.
         * @param column Zero-based column index.
         *
         * @pre Both indices must be less than kMatrix4Dimension.
         *
         * @return A mutable reference to the requested matrix element.
         */
        [[nodiscard]] constexpr auto at(std::size_t row, std::size_t column)
            -> float & {
            if (row >= kMatrix4Dimension || column >= kMatrix4Dimension) {
                Assertion::failAssertion(
                    Assertion::AssertionType::Precondition,
                    Error::Subsystem::Core,
                    "Matrix4f row or column is out of bounds");
            }
            const std::size_t index = (column * kMatrix4Dimension) + row;
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
         * @pre Both indices must be less than kMatrix4Dimension.
         *
         * @return A const reference to the requested matrix element.
         */
        [[nodiscard]] constexpr auto at(std::size_t row,
                                        std::size_t column) const -> const
            float & {
            if (row >= kMatrix4Dimension || column >= kMatrix4Dimension) {
                Assertion::failAssertion(
                    Assertion::AssertionType::Precondition,
                    Error::Subsystem::Core,
                    "Matrix4f row or column is out of bounds");
            }
            const std::size_t index = (column * kMatrix4Dimension) + row;
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
     * @return A new Matrix4f containing the product first * second.
     */
    [[nodiscard]] auto multiply(const Matrix4f &first, const Matrix4f &second)
        -> Matrix4f;

    /**
     * @brief Creates a 4x4 identity matrix.
     *
     * @return A matrix with ones on the main diagonal
     *         and zeros everywhere else.
     */
    [[nodiscard]] constexpr auto identity() -> Matrix4f {
        return Matrix4f{
            // clang-format off
            .elements = {
                1.0F, 0.0F, 0.0F, 0.0F,
                0.0F, 1.0F, 0.0F, 0.0F,
                0.0F, 0.0F, 1.0F, 0.0F,
                0.0F, 0.0F, 0.0F, 1.0F,
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
     * @return A new Matrix4f representing the specified translation.
     */
    [[nodiscard]] constexpr auto translate(float translation_x,
                                           float translation_y,
                                           float translation_z) -> Matrix4f {
        Matrix4f result = identity();
        constexpr std::size_t kTranslationColumn = kMatrix4Dimension - 1U;
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
     * @return A new Matrix4f representing the specified scaling.
     */
    [[nodiscard]] constexpr auto scale(float scale_x, float scale_y,
                                       float scale_z) -> Matrix4f {
        Matrix4f result = identity();
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
     * @return A new Matrix4f representing the Z-axis rotation.
     */
    [[nodiscard]] inline auto rotateZ(float radians) -> Matrix4f {
        Matrix4f result = identity();

        const float cosine = std::cos(radians);
        const float sine = std::sin(radians);

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
     * @return A new Matrix4f representing the X-axis rotation.
     */
    [[nodiscard]] inline auto rotateX(float radians) -> Matrix4f {
        Matrix4f result = identity();

        const float cosine = std::cos(radians);
        const float sine = std::sin(radians);

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
     * @return A new Matrix4f representing the Y-axis rotation.
     */
    [[nodiscard]] inline auto rotateY(float radians) -> Matrix4f {
        Matrix4f result = identity();

        const float cosine = std::cos(radians);
        const float sine = std::sin(radians);

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
     * @return A new Vector3f containing the transformed direction.
     */
    [[nodiscard]] constexpr auto transformDirection(const Matrix4f &matrix,
                                                    Vector3f direction)
        -> Vector3f {
        return Vector3f{
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
     * @return A new Vector3f containing the transformed position.
     */
    [[nodiscard]] constexpr auto transformPoint(const Matrix4f &matrix,
                                                Vector3f point) -> Vector3f {
        return Vector3f{
            .x = (matrix.at(0U, 0U) * point.x) + (matrix.at(0U, 1U) * point.y) +
                 (matrix.at(0U, 2U) * point.z) + matrix.at(0U, 3U),
            .y = (matrix.at(1U, 0U) * point.x) + (matrix.at(1U, 1U) * point.y) +
                 (matrix.at(1U, 2U) * point.z) + matrix.at(1U, 3U),
            .z = (matrix.at(2U, 0U) * point.x) + (matrix.at(2U, 1U) * point.y) +
                 (matrix.at(2U, 2U) * point.z) + matrix.at(2U, 3U),
        };
    }
} // namespace SNE::Engine::Core::Math
