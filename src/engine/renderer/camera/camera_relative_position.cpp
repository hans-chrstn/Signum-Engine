#include "camera_relative_position.hpp"
#include "engine/core/math/vector3.hpp"
#include <cmath>
#include <limits>
#include <optional>

namespace SNE::Engine::Renderer {
    auto
    tryMakeCameraRelativePosition(CameraRelativePositionInput input) noexcept
        -> std::optional<Core::Math::Vector3f> {
        const bool valid_world_position =
            std::isfinite(input.world_position.x) &&
            std::isfinite(input.world_position.y) &&
            std::isfinite(input.world_position.z);

        const bool valid_camera_position =
            std::isfinite(input.camera_position.x) &&
            std::isfinite(input.camera_position.y) &&
            std::isfinite(input.camera_position.z);

        if (!valid_world_position || !valid_camera_position) {
            return std::nullopt;
        }

        const Core::Math::Vector3d relative_position =
            Core::Math::subtract(input.world_position, input.camera_position);

        const bool valid_relative_position =
            std::isfinite(relative_position.x) &&
            std::isfinite(relative_position.y) &&
            std::isfinite(relative_position.z);

        if (!valid_relative_position) {
            return std::nullopt;
        }

        constexpr auto kMaximumFloat =
            static_cast<double>(std::numeric_limits<float>::max());

        const bool relative_position_in_range =
            relative_position.x >= -kMaximumFloat &&
            relative_position.x <= kMaximumFloat &&
            relative_position.y >= -kMaximumFloat &&
            relative_position.y <= kMaximumFloat &&
            relative_position.z >= -kMaximumFloat &&
            relative_position.z <= kMaximumFloat;

        if (!relative_position_in_range) {
            return std::nullopt;
        }

        return Core::Math::Vector3f{
            .x = static_cast<float>(relative_position.x),
            .y = static_cast<float>(relative_position.y),
            .z = static_cast<float>(relative_position.z),
        };
    }
} // namespace SNE::Engine::Renderer
