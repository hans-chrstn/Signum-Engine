#include "uv_sphere_mesh.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numbers>

namespace SNE::Engine::Renderer {
    auto generateUvSphereMesh(const UvSphereParameters &parameters)
        -> UvSphereMesh {

        constexpr std::uint32_t kMinimumLongitudeSegments{3U};
        constexpr std::uint32_t kMinimumLatitudeSegments{2U};

        constexpr std::size_t kMaximumVertexCount =
            static_cast<std::size_t>(
                std::numeric_limits<std::uint16_t>::max()) +
            1U;
        constexpr std::size_t kIndicesPerCell{6U};

        if (!std::isfinite(parameters.radius) || parameters.radius <= 0.0F) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Renderer,
                "UV sphere radius must be finite and greater than zero");
        }

        if (parameters.longitude_segments < kMinimumLongitudeSegments) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Renderer,
                "UV sphere longitude segment count must be at least three");
        }

        if (parameters.latitude_segments < kMinimumLatitudeSegments) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Renderer,
                "UV sphere latitude segment count must be at least two");
        }

        const auto latitude_segment_count =
            static_cast<std::size_t>(parameters.latitude_segments);
        const auto longitude_segment_count =
            static_cast<std::size_t>(parameters.longitude_segments);

        if (latitude_segment_count >= kMaximumVertexCount ||
            longitude_segment_count >= kMaximumVertexCount) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Renderer,
                "UV sphere segment count exceeds the 16-bit indexable range");
        }

        const std::size_t vertex_row_count = latitude_segment_count + 1U;
        const std::size_t vertices_per_row = longitude_segment_count + 1U;

        if (vertex_row_count > kMaximumVertexCount / vertices_per_row) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Renderer,
                "UV sphere vertex count exceeds the 16-bit indexable range");
        }

        const std::size_t vertex_count = vertex_row_count * vertices_per_row;

        const std::size_t index_count = kIndicesPerCell *
                                        (latitude_segment_count - 1U) *
                                        longitude_segment_count;

        UvSphereMesh mesh{};
        mesh.vertices.resize(vertex_count);
        mesh.indices.reserve(index_count);

        constexpr float kTwo{2.0F};
        constexpr float kPi{std::numbers::pi_v<float>};
        constexpr float kFullRotation{kTwo * kPi};

        const auto latitude_segment_count_float =
            static_cast<float>(latitude_segment_count);

        const auto longitude_segment_count_float =
            static_cast<float>(longitude_segment_count);

        for (std::size_t latitude_row{}; latitude_row <= latitude_segment_count;
             ++latitude_row) {

            const float normalized_latitude =
                static_cast<float>(latitude_row) / latitude_segment_count_float;

            const float theta = normalized_latitude * kPi;

            float cos_theta = std::cos(theta);
            float sin_theta = std::sin(theta);

            if (latitude_row == 0U) {
                cos_theta = 1.0F;
                sin_theta = 0.0F;
            } else if (latitude_row == latitude_segment_count) {
                cos_theta = -1.0F;
                sin_theta = 0.0F;
            }

            for (std::size_t longitude_column{};
                 longitude_column <= longitude_segment_count;
                 ++longitude_column) {

                const float normalized_longitude =
                    static_cast<float>(longitude_column) /
                    longitude_segment_count_float;

                const float phi = normalized_longitude * kFullRotation;

                float cos_phi = std::cos(phi);
                float sin_phi = std::sin(phi);

                if (longitude_column == longitude_segment_count) {
                    cos_phi = 1.0F;
                    sin_phi = 0.0F;
                }

                const float normal_x = sin_theta * cos_phi;
                const float normal_y = cos_theta;
                const float normal_z = sin_theta * sin_phi;

                const float tangent_x = -sin_phi;
                const float tangent_y = 0.0F;
                const float tangent_z = cos_phi;
                const float tangent_handedness = 1.0F;

                const float position_x = parameters.radius * normal_x;
                const float position_y = parameters.radius * normal_y;
                const float position_z = parameters.radius * normal_z;

                const std::size_t vertex_index =
                    (latitude_row * vertices_per_row) + longitude_column;

                mesh.vertices[vertex_index].position = {
                    position_x,
                    position_y,
                    position_z,
                };

                mesh.vertices[vertex_index].normal = {
                    normal_x,
                    normal_y,
                    normal_z,
                };

                mesh.vertices[vertex_index].uv = {
                    normalized_longitude,
                    normalized_latitude,
                };

                mesh.vertices[vertex_index].tangent = {
                    tangent_x,
                    tangent_y,
                    tangent_z,
                    tangent_handedness,
                };
            }
        }

        for (std::size_t latitude_row{}; latitude_row < latitude_segment_count;
             ++latitude_row) {

            for (std::size_t longitude_column{};
                 longitude_column < longitude_segment_count;
                 ++longitude_column) {

                const std::size_t upper_left =
                    (latitude_row * vertices_per_row) + longitude_column;
                const std::size_t upper_right = upper_left + 1U;
                const std::size_t lower_left = upper_left + vertices_per_row;
                const std::size_t lower_right = lower_left + 1U;

                const auto upper_left_index =
                    static_cast<std::uint16_t>(upper_left);
                const auto upper_right_index =
                    static_cast<std::uint16_t>(upper_right);
                const auto lower_left_index =
                    static_cast<std::uint16_t>(lower_left);
                const auto lower_right_index =
                    static_cast<std::uint16_t>(lower_right);

                if (latitude_row == 0U) {
                    mesh.indices.push_back(upper_left_index);
                    mesh.indices.push_back(lower_right_index);
                    mesh.indices.push_back(lower_left_index);
                    continue;
                }

                if (latitude_row + 1U == latitude_segment_count) {
                    mesh.indices.push_back(upper_left_index);
                    mesh.indices.push_back(upper_right_index);
                    mesh.indices.push_back(lower_left_index);
                    continue;
                }

                mesh.indices.push_back(upper_left_index);
                mesh.indices.push_back(upper_right_index);
                mesh.indices.push_back(lower_left_index);

                mesh.indices.push_back(upper_right_index);
                mesh.indices.push_back(lower_right_index);
                mesh.indices.push_back(lower_left_index);
            }
        }

        return mesh;
    }
} // namespace SNE::Engine::Renderer
