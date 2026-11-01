#include "scene_grid_geometry.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace SNE::Editor {
    namespace Assertion = SNE::Engine::Core::Assertion;
    namespace Error = SNE::Engine::Core::Error;

    auto generateSceneGridLines(SceneGridParameters parameters)
        -> std::vector<SceneGridLine> {
        constexpr std::uint32_t kMaximumGridHalfExtentCells{512U};

        if (parameters.half_extent_cells == 0U) {
            Engine::Core::Assertion::failAssertion(
                Assertion::AssertionType::Precondition,
                Error::Subsystem::Editor,
                "Scene grid half-extent must be greater than zero");
        }

        if (parameters.half_extent_cells > kMaximumGridHalfExtentCells) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition,
                Error::Subsystem::Editor,
                "Scene grid half-extent exceeds the supported maximum");
        }

        if (!std::isfinite(parameters.grid_spacing) ||
            parameters.grid_spacing <= 0.0F) {
            Engine::Core::Assertion::failAssertion(
                Assertion::AssertionType::Precondition,
                Error::Subsystem::Editor,
                "Scene grid spacing must be finite and greater than zero");
        }

        const auto half_extent_cells =
            static_cast<std::uint64_t>(parameters.half_extent_cells);

        const std::uint64_t lines_per_axis = (2U * half_extent_cells) + 1U;

        const std::uint64_t total_lines = 2U * lines_per_axis;

        std::vector<SceneGridLine> grid_lines;

        if (std::cmp_greater(total_lines, grid_lines.max_size())) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition,
                Error::Subsystem::Editor,
                "Scene grid line count exceeds maximum vector capacity");
        }

        const auto actual_grid_lines = static_cast<std::size_t>(total_lines);

        const float world_half_extent =
            static_cast<float>(parameters.half_extent_cells) *
            parameters.grid_spacing;

        if (!std::isfinite(world_half_extent)) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition,
                Error::Subsystem::Editor,
                "Scene grid world-space extent must be finite");
        }

        const auto signed_half_extent_cells =
            static_cast<std::int64_t>(half_extent_cells);

        grid_lines.reserve(actual_grid_lines);

        for (std::int64_t i{-signed_half_extent_cells};
             i <= signed_half_extent_cells; ++i) {
            const float grid_position =
                static_cast<float>(i) * parameters.grid_spacing;

            grid_lines.push_back(SceneGridLine{
                .start =
                    {
                        .x = -world_half_extent,
                        .y = 0.0F,
                        .z = grid_position,
                    },
                .end =
                    {
                        .x = world_half_extent,

                        .y = 0.0F,
                        .z = grid_position,
                    },
            });

            grid_lines.push_back(SceneGridLine{
                .start =
                    {
                        .x = grid_position,
                        .y = 0.0F,
                        .z = -world_half_extent,
                    },
                .end =
                    {
                        .x = grid_position,
                        .y = 0.0F,
                        .z = world_half_extent,
                    },
            });
        }

        return grid_lines;
    }
} // namespace SNE::Editor
