#pragma once

#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include <cmath>

namespace SNE::Engine::Core::Math {
    /**
     * @brief Represents a two-dimensional single-precision vector.
     *
     * Stores two floating-point components representing the X
     * and Y axes.
     *
     * The type is a lightweight aggregate that supports direct
     * component access and constant evaluation.
     *
     * All components are initialized to zero by default.
     *
     * This type is independent of rendering APIs and does not
     * impose SIMD-specific alignment or storage requirements.
     */
    struct Vector2f {
        /**
         * @brief Component along the X axis.
         */
        float x{};

        /**
         * @brief Component along the Y axis.
         */
        float y{};
    };

    /**
     * @brief Adds two two-dimensional vectors.
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
     * @return A new Vector2f containing the component-wise sum.
     */
    [[nodiscard]] constexpr auto add(Vector2f first, Vector2f second)
        -> Vector2f {
        return Vector2f{
            .x = first.x + second.x,
            .y = first.y + second.y,
        };
    }

    /**
     * @brief Subtracts one two-dimensional vector from another.
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
     * @return A new Vector2f containing the component-wise difference.
     */
    [[nodiscard]] constexpr auto subtract(Vector2f first, Vector2f second)
        -> Vector2f {
        return Vector2f{
            .x = first.x - second.x,
            .y = first.y - second.y,
        };
    }

    /**
     * @brief Computes the dot product of two two-dimensional vectors.
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
    [[nodiscard]] constexpr auto dot(Vector2f first, Vector2f second) -> float {
        return (first.x * second.x) + (first.y * second.y);
    }

    /**
     * @brief Computes the squared magnitude of a two-dimensional vector.
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
    [[nodiscard]] constexpr auto lengthSquared(Vector2f vector) -> float {
        return dot(vector, vector);
    }

    /**
     * @brief Computes the magnitude of a two-dimensional vector.
     *
     * Calculates the square root of the vector's squared
     * magnitude to obtain its Euclidean length.
     *
     * The input vector is not modified.
     *
     * @param vector Vector whose magnitude is calculated.
     *
     * @return The non-negative magnitude of the vector.
     */
    [[nodiscard]] inline auto length(Vector2f vector) -> float {
        return std::sqrt(lengthSquared(vector));
    }

    /**
     * @brief Normalizes a two-dimensional vector.
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
     * @return A new Vector2f containing the normalized vector.
     */
    [[nodiscard]] inline auto normalize(Vector2f vector) -> Vector2f {
        const float vector_length = length(vector);

        if (!std::isfinite(vector_length) || vector_length <= 0.0F) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition, Error::Subsystem::Core,
                "Cannot normalize a vector with an invalid magnitude");
        }

        return Vector2f{
            .x = vector.x / vector_length,
            .y = vector.y / vector_length,
        };
    }

    /**
     * @brief Scales a two-dimensional vector by a scalar value.
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
     * @return A new Vector2f containing the scaled components.
     */
    [[nodiscard]] constexpr auto scale(Vector2f vector, float scalar)
        -> Vector2f {
        return Vector2f{
            .x = vector.x * scalar,
            .y = vector.y * scalar,
        };
    }
} // namespace SNE::Engine::Core::Math
