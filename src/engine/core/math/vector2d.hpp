
#pragma once

#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include <cmath>

namespace SNE::Engine::Core::Math {
    /**
     * @brief Represents a two-dimensional double-precision vector.
     *
     * Stores two double-precision floating-point components representing
     * the X and Y axes.
     *
     * The type is a lightweight aggregate supporting direct component
     * access and constant evaluation.
     *
     * All components are initialized to zero by default.
     *
     * This type is independent of rendering APIs and does not impose
     * SIMD-specific alignment or storage requirements.
     */
    struct Vector2d {
        /**
         * @brief Component along the X axis.
         */
        double x{};

        /**
         * @brief Component along the Y axis.
         */
        double y{};
    };

    /**
     * @brief Adds two two-dimensional double-precision vectors.
     *
     * Performs component-wise addition and returns the resulting vector.
     *
     * The input vectors are not modified.
     *
     * This operation uses scalar arithmetic and supports
     * constant evaluation.
     *
     * @param first First vector operand.
     * @param second Second vector operand.
     *
     * @return A new Vector2d containing the component-wise sum.
     */
    [[nodiscard]] constexpr auto add(Vector2d first, Vector2d second)
        -> Vector2d {
        return Vector2d{
            .x = first.x + second.x,
            .y = first.y + second.y,
        };
    }

    /**
     * @brief Subtracts one two-dimensional double-precision vector
     * from another.
     *
     * Performs component-wise subtraction of the second vector
     * from the first vector.
     *
     * Subtraction is order-dependent.
     * The input vectors are not modified.
     *
     * This operation uses scalar arithmetic and supports
     * constant evaluation.
     *
     * @param first Vector to subtract from.
     * @param second Vector to subtract.
     *
     * @return A new Vector2d containing the component-wise difference.
     */
    [[nodiscard]] constexpr auto subtract(Vector2d first, Vector2d second)
        -> Vector2d {
        return Vector2d{
            .x = first.x - second.x,
            .y = first.y - second.y,
        };
    }

    /**
     * @brief Computes the dot product of two double-precision vectors.
     *
     * Multiplies corresponding components and returns the sum
     * of their products.
     *
     * The input vectors are not modified.
     *
     * This operation uses scalar arithmetic and supports
     * constant evaluation.
     *
     * @param first First vector operand.
     * @param second Second vector operand.
     *
     * @return The double-precision dot product of the two vectors.
     */
    [[nodiscard]] constexpr auto dot(Vector2d first, Vector2d second)
        -> double {
        return (first.x * second.x) + (first.y * second.y);
    }

    /**
     * @brief Computes the squared magnitude of a double-precision vector.
     *
     * Calculates the dot product of the vector with itself.
     *
     * Avoids the square root required to compute the actual magnitude.
     *
     * This operation uses scalar arithmetic and supports
     * constant evaluation.
     *
     * @param vector Vector whose squared magnitude is calculated.
     *
     * @return The squared magnitude of the vector.
     */
    [[nodiscard]] constexpr auto lengthSquared(Vector2d vector) -> double {
        return dot(vector, vector);
    }

    /**
     * @brief Computes the Euclidean magnitude of a double-precision vector.
     *
     * Calculates the vector's magnitude using std::hypot to reduce
     * unnecessary intermediate overflow and underflow.
     *
     * The input vector is not modified.
     *
     * @param vector Vector whose magnitude is calculated.
     *
     * @return The non-negative magnitude of the vector.
     */

    [[nodiscard]] inline auto length(Vector2d vector) -> double {
        return std::hypot(vector.x, vector.y);
    }

    /**
     * @brief Normalizes a two-dimensional double-precision vector.
     *
     * Divides each component by the vector's magnitude to produce
     * a unit-length vector preserving the original direction.
     *
     * The input vector is not modified.
     *
     * A precondition assertion is triggered if the calculated
     * magnitude is zero or non-finite.
     *
     * @param vector Vector to normalize.
     *
     * @pre The calculated magnitude must be finite and greater
     *      than zero.
     *
     * @return A new Vector2d containing the normalized vector.
     */
    [[nodiscard]] inline auto normalize(Vector2d vector) -> Vector2d {
        const double vector_length = length(vector);

        if (!std::isfinite(vector_length) || vector_length <= 0.0) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition, Error::Subsystem::Core,
                "Cannot normalize a vector with an invalid magnitude");
        }

        return Vector2d{
            .x = vector.x / vector_length,
            .y = vector.y / vector_length,
        };
    }

    /**
     * @brief Scales a two-dimensional double-precision vector.
     *
     * Multiplies each component by the supplied double-precision scalar.
     *
     * A scalar of one preserves the vector.
     * A scalar of zero produces a zero vector.
     * A negative scalar reverses the direction of a nonzero vector.
     *
     * The input vector is not modified.
     *
     * This operation uses scalar arithmetic and supports
     * constant evaluation.
     *
     * @param vector Vector to scale.
     * @param scalar Double-precision scalar multiplier.
     *
     * @return A new Vector2d containing the scaled components.
     */
    [[nodiscard]] constexpr auto scale(Vector2d vector, double scalar)
        -> Vector2d {
        return Vector2d{
            .x = vector.x * scalar,
            .y = vector.y * scalar,
        };
    }

} // namespace SNE::Engine::Core::Math
