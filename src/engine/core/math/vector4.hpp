#pragma once

#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include <cmath>

namespace SNE::Engine::Core::Math {
    /**
     * @brief Represents a four-dimensional single-precision floating-point
     * vector.
     *
     * Stores four components representing coordinates or values
     * in four-dimensional space.
     *
     * The vector supports addition, subtraction, dot products,
     * magnitude calculations, normalization, and scalar multiplication.
     *
     * All four components participate in mathematical operations.
     *
     * Default initialization produces a zero vector.
     *
     * The W component has no implicit homogeneous-coordinate semantics.
     * This type is independent of rendering APIs.
     */
    struct Vector4f {
        /**
         * @brief X component of the vector.
         *
         * Initialized to zero by default.
         */
        float x{};

        /**
         * @brief Y component of the vector.
         *
         * Initialized to zero by default.
         */
        float y{};

        /**
         * @brief Z component of the vector.
         *
         * Initialized to zero by default.
         */
        float z{};

        /**
         * @brief W component of the vector.
         *
         * Initialized to zero by default.
         */
        float w{};
    };

    /**
     * @brief Adds two four-dimensional vectors.
     *
     * Computes the component-wise sum of the provided vectors.
     *
     * Each component of the result is the sum of the corresponding
     * components of the input vectors.
     *
     * The input vectors are not modified.
     *
     * @param first First vector operand.
     * @param second Second vector operand.
     *
     * @return A new Vector4f containing the component-wise sum.
     */
    [[nodiscard]] constexpr auto add(Vector4f first, Vector4f second)
        -> Vector4f {
        return Vector4f{
            .x = first.x + second.x,
            .y = first.y + second.y,
            .z = first.z + second.z,
            .w = first.w + second.w,
        };
    }

    /**
     * @brief Subtracts one four-dimensional vector from another.
     *
     * Computes the component-wise difference first - second.
     *
     * Each component of the result is obtained by subtracting
     * the corresponding component of the second vector from
     * the first vector.
     *
     * The input vectors are not modified.
     *
     * @param first Vector to subtract from.
     * @param second Vector to subtract.
     *
     * @return A new Vector4f containing the component-wise difference.
     */
    [[nodiscard]] constexpr auto subtract(Vector4f first, Vector4f second)
        -> Vector4f {
        return Vector4f{
            .x = first.x - second.x,
            .y = first.y - second.y,
            .z = first.z - second.z,
            .w = first.w - second.w,
        };
    }

    /**
     * @brief Computes the dot product of two four-dimensional vectors.
     *
     * Multiplies corresponding components of the input vectors
     * and returns the sum of those products.
     *
     * All four components contribute to the result.
     *
     * The dot product is zero for perpendicular vectors
     * under the standard Euclidean inner product.
     *
     * @param first First vector operand.
     * @param second Second vector operand.
     *
     * @return The scalar dot product of the two vectors.
     */
    [[nodiscard]] constexpr auto dot(Vector4f first, Vector4f second) -> float {
        return (first.x * second.x) + (first.y * second.y) +
               (first.z * second.z) + (first.w * second.w);
    }

    /**
     * @brief Computes the squared Euclidean length of a vector.
     *
     * Calculates the dot product of the vector with itself.
     *
     * Avoids the square root operation required to compute
     * the actual vector length.
     *
     * Useful for comparing vector magnitudes when the
     * actual length is unnecessary.
     *
     * @param vector Vector whose squared length is calculated.
     *
     * @return The squared Euclidean length of the vector.
     */
    [[nodiscard]] constexpr auto lengthSquared(Vector4f vector) -> float {
        return dot(vector, vector);
    }

    /**
     * @brief Computes the Euclidean length of a vector.
     *
     * Calculates the square root of the sum of the squares
     * of all four vector components.
     *
     * The resulting value represents the magnitude
     * of the vector in four-dimensional Euclidean space.
     *
     * @param vector Vector whose length is calculated.
     *
     * @return The Euclidean length of the vector.
     */
    [[nodiscard]] inline auto length(Vector4f vector) -> float {
        return std::sqrt(lengthSquared(vector));
    }

    /**
     * @brief Normalizes a four-dimensional vector to unit length.
     *
     * Divides each component of the vector by its Euclidean length.
     *
     * For a valid nonzero vector, the resulting vector has
     * approximately unit length and preserves the original direction,
     * subject to floating-point precision.
     *
     * Normalization requires a positive, finite computed magnitude.
     * An invalid magnitude triggers a precondition assertion.
     *
     * The input vector is not modified.
     *
     * @param vector Vector to normalize.
     *
     * @pre The computed vector length must be greater than zero
     *      and finite.
     *
     * @return A new Vector4f containing the normalized vector.
     */
    [[nodiscard]] inline auto normalize(Vector4f vector) -> Vector4f {
        const float vector_length = length(vector);
        if (vector_length <= 0.0F || !std::isfinite(vector_length)) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition, Error::Subsystem::Core,
                "Cannot normalize a vector with an invalid magnitude");
        }

        return Vector4f{
            .x = vector.x / vector_length,
            .y = vector.y / vector_length,
            .z = vector.z / vector_length,
            .w = vector.w / vector_length,
        };
    }

    /**
     * @brief Multiplies a four-dimensional vector by a scalar.
     *
     * Computes the component-wise product of the vector
     * and the provided scalar.
     *
     * A scalar of 1 preserves the vector.
     * A scalar of 0 produces a zero vector.
     * A negative scalar reverses the direction of a nonzero vector.
     *
     * The input vector is not modified.
     *
     * @param vector Vector to scale.
     * @param scalar Scalar multiplication factor.
     *
     * @return A new Vector4f containing the scaled vector.
     */
    [[nodiscard]] constexpr auto scale(Vector4f vector, float scalar)
        -> Vector4f {
        return Vector4f{
            .x = vector.x * scalar,
            .y = vector.y * scalar,
            .z = vector.z * scalar,
            .w = vector.w * scalar,
        };
    }
} // namespace SNE::Engine::Core::Math
