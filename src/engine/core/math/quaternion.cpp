#include "quaternion.hpp"
#include "matrix4.hpp"

#include <cstddef>

namespace SNE::Engine::Core::Math {
    auto toRotationMatrix(Quaternionf quaternion) -> Matrix4f {
        const Quaternionf normalized_quaternion = normalize(quaternion);

        constexpr float kRotationFactor = 2.0F;

        constexpr std::size_t kXIndex = 0U;
        constexpr std::size_t kYIndex = 1U;
        constexpr std::size_t kZIndex = 2U;

        const float x_squared =
            normalized_quaternion.x * normalized_quaternion.x;
        const float y_squared =
            normalized_quaternion.y * normalized_quaternion.y;
        const float z_squared =
            normalized_quaternion.z * normalized_quaternion.z;

        const float xy_product =
            normalized_quaternion.x * normalized_quaternion.y;
        const float xz_product =
            normalized_quaternion.x * normalized_quaternion.z;
        const float yz_product =
            normalized_quaternion.y * normalized_quaternion.z;

        const float xw_product =
            normalized_quaternion.x * normalized_quaternion.w;
        const float yw_product =
            normalized_quaternion.y * normalized_quaternion.w;
        const float zw_product =
            normalized_quaternion.z * normalized_quaternion.w;

        Matrix4f result = identity();

        result.at(kXIndex, kXIndex) -=
            kRotationFactor * (y_squared + z_squared);

        result.at(kXIndex, kYIndex) =
            kRotationFactor * (xy_product - zw_product);

        result.at(kXIndex, kZIndex) =
            kRotationFactor * (xz_product + yw_product);

        result.at(kYIndex, kXIndex) =
            kRotationFactor * (xy_product + zw_product);

        result.at(kYIndex, kYIndex) -=
            kRotationFactor * (x_squared + z_squared);

        result.at(kYIndex, kZIndex) =
            kRotationFactor * (yz_product - xw_product);

        result.at(kZIndex, kXIndex) =
            kRotationFactor * (xz_product - yw_product);

        result.at(kZIndex, kYIndex) =
            kRotationFactor * (yz_product + xw_product);

        result.at(kZIndex, kZIndex) -=
            kRotationFactor * (x_squared + y_squared);

        return result;
    }
} // namespace SNE::Engine::Core::Math
