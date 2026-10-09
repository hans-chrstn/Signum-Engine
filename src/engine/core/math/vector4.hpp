#pragma once

#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include <cmath>
namespace SNE::Engine::Core::Math {
    struct Vector4f {
        float x{};

        float y{};

        float z{};

        float w{};
    };

    [[nodiscard]] constexpr auto add(Vector4f first, Vector4f second)
        -> Vector4f {
        return Vector4f{
            .x = first.x + second.x,
            .y = first.y + second.y,
            .z = first.z + second.z,
            .w = first.w + second.w,
        };
    }

    [[nodiscard]] constexpr auto subtract(Vector4f first, Vector4f second)
        -> Vector4f {
        return Vector4f{
            .x = first.x - second.x,
            .y = first.y - second.y,
            .z = first.z - second.z,
            .w = first.w - second.w,
        };
    }

    [[nodiscard]] constexpr auto dot(Vector4f first, Vector4f second) -> float {
        return (first.x * second.x) + (first.y * second.y) +
               (first.z * second.z) + (first.w * second.w);
    }

    [[nodiscard]] constexpr auto lengthSquared(Vector4f vector) -> float {
        return dot(vector, vector);
    }

    [[nodiscard]] inline auto length(Vector4f vector) -> float {
        return std::sqrt(lengthSquared(vector));
    }

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
