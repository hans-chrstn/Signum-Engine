
#pragma once

#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/core/math/matrix4.hpp"
#include "engine/core/math/vector3.hpp"
#include <cmath>

namespace SNE::Engine::Core::Math {
    /**
     * @brief Represents a single-precision quaternion.
     *
     * Stores four floating-point components consisting of a vector
     * part (x, y, z) and a scalar part (w).
     *
     * A unit-length quaternion can represent a three-dimensional
     * rotation using the right-handed Hamilton convention.
     *
     * Quaternion multiplication follows the column-vector rotation
     * composition convention used by Matrix4f.
     *
     * Default initialization produces a zero quaternion, not an
     * identity rotation.
     *
     * This type does not enforce unit length and can also represent
     * general mathematical quaternions.
     *
     * This type is independent of rendering APIs and does not
     * impose SIMD-specific alignment or storage requirements.
     */
    struct Quaternionf {
        /**
         * @brief X component of the quaternion vector part.
         */
        float x{};

        /**
         * @brief Y component of the quaternion vector part.
         */
        float y{};

        /**
         * @brief Z component of the quaternion vector part.
         */
        float z{};

        /**
         * @brief Scalar component of the quaternion.
         */
        float w{};
    };

    /**
     * @brief Creates an identity quaternion.
     *
     * Produces the quaternion (0, 0, 0, 1), representing no
     * rotation.
     *
     * The resulting quaternion has unit length and acts as the
     * identity element of Hamilton multiplication.
     *
     * @return A unit quaternion representing the identity rotation.
     */
    [[nodiscard]] constexpr auto quaternionIdentity() -> Quaternionf {
        return Quaternionf{
            .x = 0.0F,
            .y = 0.0F,
            .z = 0.0F,
            .w = 1.0F,
        };
    }

    /**
     * @brief Adds two quaternions component-wise.
     *
     * Adds corresponding x, y, z, and w components of the
     * input quaternions.
     *
     * Quaternion addition does not compose rotations.
     * Adding two unit quaternions does not generally produce
     * another unit quaternion.
     *
     * The input quaternions are not modified.
     *
     * This operation supports constant evaluation.
     *
     * @param first First quaternion operand.
     * @param second Second quaternion operand.
     *
     * @return A new Quaternionf containing the component-wise sum.
     */
    [[nodiscard]] constexpr auto add(Quaternionf first, Quaternionf second)
        -> Quaternionf {
        return Quaternionf{
            .x = first.x + second.x,
            .y = first.y + second.y,
            .z = first.z + second.z,
            .w = first.w + second.w,
        };
    }

    /**
     * @brief Subtracts one quaternion from another component-wise.
     *
     * Subtracts the corresponding components of the second
     * quaternion from the first quaternion.
     *
     * Quaternion subtraction is order-dependent and does not
     * directly compute a relative rotation.
     *
     * The input quaternions are not modified.
     *
     * This operation supports constant evaluation.
     *
     * @param first Quaternion to subtract from.
     * @param second Quaternion to subtract.
     *
     * @return A new Quaternionf containing the component-wise
     *         difference.
     */
    [[nodiscard]] constexpr auto subtract(Quaternionf first, Quaternionf second)
        -> Quaternionf {
        return Quaternionf{
            .x = first.x - second.x,
            .y = first.y - second.y,
            .z = first.z - second.z,
            .w = first.w - second.w,
        };
    }

    /**
     * @brief Computes the four-dimensional dot product of two quaternions.
     *
     * Multiplies corresponding components of the input quaternions
     * and returns the sum of those products.
     *
     * The dot product is useful for quaternion interpolation,
     * magnitude calculations, and orientation comparisons.
     *
     * Opposite unit quaternions represent the same rotation,
     * despite having a dot product of negative one.
     *
     * The input quaternions are not modified.
     *
     * This operation supports constant evaluation.
     *
     * @param first First quaternion operand.
     * @param second Second quaternion operand.
     *
     * @return The scalar dot product of the two quaternions.
     */
    [[nodiscard]] constexpr auto dot(Quaternionf first, Quaternionf second)
        -> float {
        return (first.x * second.x) + (first.y * second.y) +
               (first.z * second.z) + (first.w * second.w);
    }

    /**
     * @brief Computes the squared magnitude of a quaternion.
     *
     * Calculates the sum of the squares of all four quaternion
     * components using the quaternion dot product.
     *
     * Avoids the square root required by length().
     *
     * Floating-point overflow or underflow may occur for
     * extreme component magnitudes.
     *
     * This operation supports constant evaluation.
     *
     * @param quaternion Quaternion whose squared magnitude
     *                   is calculated.
     *
     * @return The squared magnitude of the quaternion.
     */
    [[nodiscard]] constexpr auto lengthSquared(Quaternionf quaternion)
        -> float {
        return dot(quaternion, quaternion);
    }

    /**
     * @brief Computes the magnitude of a quaternion.
     *
     * Calculates the square root of the quaternion's squared
     * magnitude.
     *
     * A quaternion representing a rotation should normally
     * have a magnitude of approximately one.
     *
     * Floating-point overflow or underflow in the squared
     * magnitude calculation can affect the result.
     *
     * @param quaternion Quaternion whose magnitude is calculated.
     *
     * @return The magnitude of the quaternion.
     */
    [[nodiscard]] inline auto length(Quaternionf quaternion) -> float {
        return std::sqrt(lengthSquared(quaternion));
    }

    /**
     * @brief Normalizes a quaternion to unit length.
     *
     * Divides all four components by the quaternion's magnitude.
     *
     * Normalization produces a unit quaternion suitable for
     * representing a three-dimensional rotation when the
     * arithmetic result is valid.
     *
     * The input quaternion is not modified.
     *
     * The function triggers a precondition assertion if the
     * calculated magnitude is zero, negative, or non-finite.
     *
     * The implementation does not guarantee numerical robustness
     * for all extreme floating-point magnitudes.
     *
     * @param quaternion Quaternion to normalize.
     *
     * @pre The calculated magnitude must be finite and
     *      greater than zero.
     *
     * @return A new Quaternionf containing the normalized
     *         quaternion.
     */
    [[nodiscard]] inline auto normalize(Quaternionf quaternion) -> Quaternionf {
        const float quaternion_length = length(quaternion);
        if (quaternion_length <= 0.0F || !std::isfinite(quaternion_length)) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition, Error::Subsystem::Core,
                "Cannot normalize a quaternion with an invalid magnitude");
        }

        return Quaternionf{
            .x = quaternion.x / quaternion_length,
            .y = quaternion.y / quaternion_length,
            .z = quaternion.z / quaternion_length,
            .w = quaternion.w / quaternion_length,
        };
    }

    /**
     * @brief Multiplies every quaternion component by a scalar.
     *
     * Scales the x, y, z, and w components independently by
     * the same scalar value.
     *
     * Scalar multiplication does not directly compose rotations.
     * Scaling a unit quaternion generally changes its magnitude.
     *
     * The input quaternion is not modified.
     *
     * This operation supports constant evaluation.
     *
     * @param quaternion Quaternion to scale.
     * @param scalar Scalar multiplier applied to all components.
     *
     * @return A new Quaternionf containing the scaled quaternion.
     */
    [[nodiscard]] constexpr auto scale(Quaternionf quaternion, float scalar)
        -> Quaternionf {
        return Quaternionf{
            .x = quaternion.x * scalar,
            .y = quaternion.y * scalar,
            .z = quaternion.z * scalar,
            .w = quaternion.w * scalar,
        };
    }

    /**
     * @brief Computes the Hamilton product of two quaternions.
     *
     * Performs quaternion multiplication using the right-handed
     * Hamilton convention.
     *
     * Unlike component-wise multiplication, the Hamilton product
     * combines quaternion vector and scalar components.
     *
     * For unit quaternions representing rotations, the product
     * first * second applies the rotation represented by second
     * before the rotation represented by first.
     *
     * This ordering matches the column-vector transformation
     * convention used by Matrix4f.
     *
     * Quaternion multiplication is associative but generally
     * not commutative.
     *
     * The input quaternions are not modified.
     *
     * This operation supports constant evaluation.
     *
     * @param first Left-hand quaternion operand.
     * @param second Right-hand quaternion operand.
     *
     * @return A new Quaternionf containing the Hamilton product.
     */
    [[nodiscard]] constexpr auto multiply(Quaternionf first, Quaternionf second)
        -> Quaternionf {
        return Quaternionf{
            .x = (first.w * second.x) + (first.x * second.w) +
                 (first.y * second.z) - (first.z * second.y),
            .y = (first.w * second.y) - (first.x * second.z) +
                 (first.y * second.w) + (first.z * second.x),
            .z = (first.w * second.z) + (first.x * second.y) -
                 (first.y * second.x) + (first.z * second.w),
            .w = (first.w * second.w) - (first.x * second.x) -
                 (first.y * second.y) - (first.z * second.z),
        };
    }

    /**
     * @brief Creates a quaternion from an axis and rotation angle.
     *
     * Constructs a quaternion representing a right-handed
     * rotation around the specified three-dimensional axis.
     *
     * The rotation axis is normalized before calculating
     * the quaternion components.
     *
     * The vector part is calculated using the normalized axis
     * multiplied by the sine of half the rotation angle.
     * The scalar part is the cosine of half the rotation angle.
     *
     * Rotation angles are expressed in radians.
     *
     * The function triggers a precondition assertion for a
     * non-finite angle or an axis that cannot be normalized.
     *
     * @param axis Axis around which the rotation is performed.
     * @param angle_radians Rotation angle expressed in radians.
     *
     * @pre The rotation angle must be finite.
     * @pre The rotation axis must have a finite, nonzero
     *      calculated magnitude.
     *
     * @return A new Quaternionf representing the specified
     *         axis-angle rotation.
     */
    [[nodiscard]] inline auto fromAxisAngle(Vector3f axis, float angle_radians)
        -> Quaternionf {
        if (!std::isfinite(angle_radians)) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition, Error::Subsystem::Core,
                "Cannot create a quaternion from a non-finite angle");
        }

        const Vector3f normalized_axis = normalize(axis);

        const float half_angle = angle_radians * 0.5F;

        const float sine = std::sin(half_angle);
        const float cosine = std::cos(half_angle);

        return Quaternionf{
            .x = normalized_axis.x * sine,
            .y = normalized_axis.y * sine,
            .z = normalized_axis.z * sine,
            .w = cosine,
        };
    }

    /**
     * @brief Converts a quaternion into a 4x4 rotation matrix.
     *
     * Normalizes the input quaternion and constructs a matrix
     * representing the corresponding three-dimensional rotation.
     *
     * The resulting matrix uses column-major storage and the
     * column-vector transformation convention of Matrix4f.
     *
     * The upper-left 3x3 portion contains the rotation.
     * Translation components are zero, and the bottom-right
     * element is one.
     *
     * Opposite quaternions represent the same rotation and
     * therefore produce equivalent rotation matrices.
     *
     * The function triggers a precondition assertion if the
     * input quaternion cannot be normalized.
     *
     * @param quaternion Quaternion to convert into a rotation matrix.
     *
     * @pre The quaternion must have a finite, nonzero
     *      calculated magnitude.
     *
     * @return A Matrix4f representing the quaternion rotation.
     */
    [[nodiscard]] auto toRotationMatrix(Quaternionf quaternion) -> Matrix4f;
} // namespace SNE::Engine::Core::Math
