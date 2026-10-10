#pragma once

#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include <cmath>
namespace SNE::Engine::Core::Math {

    /**
     * @brief Represents a three-dimensional double-precision vector.
     *
     * Stores three double-precision floating-point components representing
     * the X, Y, and Z axes.
     *
     * The type is a lightweight aggregate that supports direct component
     * access and constant evaluation.
     *
     * All components are initialized to zero by default.
     *
     * This type is independent of rendering APIs and does not impose
     * SIMD-specific alignment or storage requirements.
     */
    struct Vector3d {
        /**
         * @brief Component along the X axis.
         */
        double x{};

        /**
         * @brief Component along the Y axis.
         */
        double y{};

        /**
         * @brief Component along the Z axis.
         */
        double z{};
    };

    /**
     * @brief Adds two three-dimensional vectors.
     *
     * Performs component-wise addition of the provided vectors
     * and returns the resulting vector.
     *
     * The input vectors are not modified.
     *
     * This operation uses scalar arithmetic and supports
     * constant evaluation.
     *
     * @param first First vector operand.
     * @param second Second vector operand.
     *
     * @return A new Vector3d containing the component-wise sum.
     */
    [[nodiscard]] constexpr auto add(Vector3d first, Vector3d second)
        -> Vector3d {
        return Vector3d{
            .x = first.x + second.x,
            .y = first.y + second.y,
            .z = first.z + second.z,
        };
    }

    /**
     * @brief Subtracts one three-dimensional vector from another.
     *
     * Performs component-wise subtraction of the second vector
     * from the first vector and returns the result.
     *
     * Vector subtraction is order-dependent.
     * The input vectors are not modified.
     *
     * This operation uses scalar arithmetic and supports
     * constant evaluation.
     *
     * @param first Vector to subtract from.
     * @param second Vector to subtract.
     *
     * @return A new Vector3d containing the component-wise difference.
     */
    [[nodiscard]] constexpr auto subtract(Vector3d first, Vector3d second)
        -> Vector3d {
        return Vector3d{
            .x = first.x - second.x,
            .y = first.y - second.y,
            .z = first.z - second.z,
        };
    }

    /**
     * @brief Computes the dot product of two three-dimensional vectors.
     *
     * Multiplies corresponding components of the input vectors
     * and returns the sum of those products.
     *
     * The result is a scalar value representing the dot product.
     * The input vectors are not modified.
     *
     * This operation uses scalar arithmetic and supports
     * constant evaluation.
     *
     * @param first First vector operand.
     * @param second Second vector operand.
     *
     * @return The dot product of the two vectors.
     */
    [[nodiscard]] constexpr auto dot(Vector3d first, Vector3d second)
        -> double {
        return (first.x * second.x) + (first.y * second.y) +
               (first.z * second.z);
    }

    /**
     * @brief Computes the cross product of two three-dimensional vectors.
     *
     * Computes a vector perpendicular to both input vectors
     * using the right-handed cross product convention.
     *
     * The operation is anti-commutative:
     * cross(first, second) = -cross(second, first).
     *
     * The input vectors are not modified.
     * This operation uses scalar arithmetic and supports
     * constant evaluation.
     *
     * @param first First vector operand.
     * @param second Second vector operand.
     *
     * @return A new Vector3d containing the cross product.
     */
    [[nodiscard]] constexpr auto cross(Vector3d first, Vector3d second)
        -> Vector3d {
        return Vector3d{
            .x = (first.y * second.z) - (first.z * second.y),
            .y = (first.z * second.x) - (first.x * second.z),
            .z = (first.x * second.y) - (first.y * second.x),
        };
    }

    /**
     * @brief Computes the squared magnitude of a three-dimensional vector.
     *
     * Calculates the dot product of the vector with itself,
     * avoiding the square root required for the actual magnitude.
     *
     * This operation uses scalar arithmetic and supports
     * constant evaluation.
     *
     * @param vector Vector whose squared magnitude is calculated.
     *
     * @return The squared magnitude of the vector.
     */
    [[nodiscard]] constexpr auto lengthSquared(Vector3d vector) -> double {
        return dot(vector, vector);
    }

    /**
     * @brief Computes the Euclidean magnitude of a three-dimensional vector.
     *
     * Calculates the vector's magnitude using std::hypot to reduce
     * unnecessary intermediate overflow and underflow.
     *
     * All three vector components contribute to the magnitude.
     *
     * The input vector is not modified.
     *
     * @param vector Vector whose magnitude is calculated.
     *
     * @return The non-negative magnitude of the vector.
     */
    [[nodiscard]] inline auto length(Vector3d vector) -> double {
        return std::hypot(vector.x, vector.y, vector.z);
    }

    /**
     * @brief Normalizes a three-dimensional vector.
     *
     * Divides each component by the vector's magnitude,
     * producing a unit-length vector with the same direction.
     *
     * The input vector is not modified.
     *
     * A precondition assertion is triggered if the magnitude
     * is zero or non-finite.
     *
     * @param vector Vector to normalize.
     *
     * @return A new Vector3d containing the normalized vector.
     */
    [[nodiscard]] inline auto normalize(Vector3d vector) -> Vector3d {
        const double vector_length = length(vector);
        if (!std::isfinite(vector_length) || vector_length <= 0.0) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition, Error::Subsystem::Core,
                "Cannot normalize a vector with an invalid magnitude");
        }

        return Vector3d{
            .x = vector.x / vector_length,
            .y = vector.y / vector_length,
            .z = vector.z / vector_length,
        };
    }

    /**
     * @brief Scales a three-dimensional vector by a scalar value.
     *
     * Multiplies each component of the vector by the provided
     * scalar and returns the resulting vector.
     *
     * The input vector is not modified.
     *
     * This operation uses scalar arithmetic and supports
     * constant evaluation.
     *
     * @param vector Vector to scale.
     * @param scalar Scalar multiplier.
     *
     * @return A new Vector3d containing the scaled components.
     */
    [[nodiscard]] constexpr auto scale(Vector3d vector, double scalar)
        -> Vector3d {
        return Vector3d{
            .x = vector.x * scalar,
            .y = vector.y * scalar,
            .z = vector.z * scalar,
        };
    }
} // namespace SNE::Engine::Core::Math
