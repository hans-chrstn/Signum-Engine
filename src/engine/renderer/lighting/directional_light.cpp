#include "directional_light.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include <cmath>

namespace SNE::Engine::Renderer {
    auto validateDirectionalLight(const DirectionalLight &light) -> void {
        const Core::Math::Vector3f &direction = light.direction_to_light;
        const Core::Math::Vector3f &color = light.color;
        const float intensity = light.intensity;

        if (!std::isfinite(direction.x) || !std::isfinite(direction.y) ||
            !std::isfinite(direction.z) ||
            (direction.x == 0.0F && direction.y == 0.0F &&
             direction.z == 0.0F)) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Renderer,
                "DirectionalLight requires a finite, nonzero direction");
        }

        if (!std::isfinite(color.x) || !std::isfinite(color.y) ||
            !std::isfinite(color.z) || color.x < 0.0F || color.y < 0.0F ||
            color.z < 0.0F) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Renderer,
                "DirectionalLight requires finite, nonnegative color "
                "components");
        }

        if (!std::isfinite(intensity) || intensity < 0.0F) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Renderer,
                "DirectionalLight requires finite, nonnegative intensity");
        }
    }
} // namespace SNE::Engine::Renderer
