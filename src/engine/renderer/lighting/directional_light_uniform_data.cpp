#include "directional_light_uniform_data.hpp"
#include "engine/core/math/vector3.hpp"
#include <algorithm>
#include <cmath>

namespace SNE::Engine::Renderer {

    auto makeDirectionalLightUniformData(const DirectionalLight &light)
        -> DirectionalLightUniformData {

        validateDirectionalLight(light);

        const Core::Math::Vector3f &direction = light.direction_to_light;

        const float maximum_component = std::max({
            std::fabs(direction.x),
            std::fabs(direction.y),
            std::fabs(direction.z),
        });

        const Core::Math::Vector3f scaled{
            .x = direction.x / maximum_component,
            .y = direction.y / maximum_component,
            .z = direction.z / maximum_component,
        };

        const Core::Math::Vector3f normalized_direction =
            Core::Math::normalize(scaled);

        return DirectionalLightUniformData{
            .direction =
                {
                    .x = normalized_direction.x,
                    .y = normalized_direction.y,
                    .z = normalized_direction.z,
                    .w = 0.0F,
                },
            .color_intensity =
                {
                    .x = light.color.x,
                    .y = light.color.y,
                    .z = light.color.z,
                    .w = light.intensity,
                },
        };
    }
} // namespace SNE::Engine::Renderer
