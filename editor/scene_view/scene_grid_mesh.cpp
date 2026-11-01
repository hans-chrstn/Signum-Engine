#include "scene_grid_mesh.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "scene_view/scene_grid_geometry.hpp"
#include <cstddef>
#include <cstdint>
#include <limits>

namespace SNE::Editor {
    namespace Assertion = Engine::Core::Assertion;
    namespace Error = Engine::Core::Error;

    auto makeSceneGridMesh(std::span<const SceneGridLine> lines)
        -> SceneGridMesh {
        SceneGridMesh mesh{};

        constexpr std::size_t kMaximumLines =
            (static_cast<std::size_t>(
                 std::numeric_limits<std::uint16_t>::max()) +
             1U) /
            2U;

        if (lines.size() > kMaximumLines) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition,
                Error::Subsystem::Editor,
                "Scene grid mesh line count exceeds 16-bit index capacity");
        }

        const std::size_t vertex_count = lines.size() * 2U;

        mesh.vertices.reserve(vertex_count);
        mesh.indices.reserve(vertex_count);

        for (const SceneGridLine &line : lines) {
            const SceneGridVertex start{
                .position = {line.start.x, line.start.y, line.start.z},
            };

            const SceneGridVertex end{
                .position = {line.end.x, line.end.y, line.end.z},
            };

            const std::size_t grid_vertex_count{mesh.vertices.size()};

            mesh.vertices.push_back(start);
            mesh.vertices.push_back(end);

            const std::size_t first_vertex_index = grid_vertex_count;
            const std::size_t second_vertex_index = first_vertex_index + 1U;

            mesh.indices.push_back(
                static_cast<std::uint16_t>(first_vertex_index));

            mesh.indices.push_back(
                static_cast<std::uint16_t>(second_vertex_index));
        }

        return mesh;
    }
} // namespace SNE::Editor
