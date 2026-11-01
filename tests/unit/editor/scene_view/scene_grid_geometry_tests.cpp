#include "scene_view/scene_grid_geometry.hpp"
#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>
#include <limits>

namespace Editor = SNE::Editor;

TEST(SceneGridGeometryTests, GeneratesExpectedLineCounts) {
    for (const std::uint32_t half_extent : {1U, 2U, 10U, 512U}) {
        SCOPED_TRACE(half_extent);

        const Editor::SceneGridParameters parameters{
            .half_extent_cells = half_extent,
            .grid_spacing = 1.0F,
        };

        const auto lines = Editor::generateSceneGridLines(parameters);

        const std::size_t expected_count =
            (4U * static_cast<std::size_t>(half_extent)) + 2U;

        EXPECT_EQ(lines.size(), expected_count);
    }
}

TEST(SceneGridGeometryTests, GeneratesCorrectEndpoints) {
    const Editor::SceneGridParameters parameters{
        .half_extent_cells = 1U,
        .grid_spacing = 1.5F,
    };

    const auto lines = Editor::generateSceneGridLines(parameters);

    ASSERT_EQ(lines.size(), 6U);

    // First X-parallel line, at Z = -1.5.
    EXPECT_FLOAT_EQ(lines[0].start.x, -1.5F);
    EXPECT_FLOAT_EQ(lines[0].end.x, 1.5F);
    EXPECT_FLOAT_EQ(lines[0].start.z, -1.5F);
    EXPECT_FLOAT_EQ(lines[0].end.z, -1.5F);

    // First Z-parallel line, at X = -1.5.
    EXPECT_FLOAT_EQ(lines[1].start.x, -1.5F);
    EXPECT_FLOAT_EQ(lines[1].end.x, -1.5F);
    EXPECT_FLOAT_EQ(lines[1].start.z, -1.5F);
    EXPECT_FLOAT_EQ(lines[1].end.z, 1.5F);

    // Both center lines must cross the origin.
    EXPECT_FLOAT_EQ(lines[2].start.z, 0.0F);
    EXPECT_FLOAT_EQ(lines[2].end.z, 0.0F);
    EXPECT_FLOAT_EQ(lines[3].start.x, 0.0F);
    EXPECT_FLOAT_EQ(lines[3].end.x, 0.0F);

    // The grid must lie entirely on the XZ plane.
    for (const auto &line : lines) {
        EXPECT_FLOAT_EQ(line.start.y, 0.0F);
        EXPECT_FLOAT_EQ(line.end.y, 0.0F);
    }
}

TEST(SceneGridGeometryTests, RejectsZeroHalfExtent) {
    const Editor::SceneGridParameters parameters{
        .half_extent_cells = 0U,
        .grid_spacing = 1.0F,
    };

    ASSERT_DEATH(static_cast<void>(Editor::generateSceneGridLines(parameters)),
                 "Scene grid half-extent must be greater than zero");
}

TEST(SceneGridGeometryTests, RejectsExcessiveHalfExtent) {
    const Editor::SceneGridParameters parameters{
        .half_extent_cells = 513U,
        .grid_spacing = 1.0F,
    };

    ASSERT_DEATH(static_cast<void>(Editor::generateSceneGridLines(parameters)),
                 "Scene grid half-extent exceeds the supported maximum");
}

TEST(SceneGridGeometryTests, RejectsInvalidSpacing) {
    Editor::SceneGridParameters parameters{
        .half_extent_cells = 2U,
        .grid_spacing = 0.0F,
    };

    constexpr auto kExpectedMessage =
        "Scene grid spacing must be finite and greater than zero";

    ASSERT_DEATH(static_cast<void>(Editor::generateSceneGridLines(parameters)),
                 kExpectedMessage);

    parameters.grid_spacing = -1.0F;
    ASSERT_DEATH(static_cast<void>(Editor::generateSceneGridLines(parameters)),
                 kExpectedMessage);

    parameters.grid_spacing = std::numeric_limits<float>::infinity();
    ASSERT_DEATH(static_cast<void>(Editor::generateSceneGridLines(parameters)),
                 kExpectedMessage);

    parameters.grid_spacing = std::numeric_limits<float>::quiet_NaN();
    ASSERT_DEATH(static_cast<void>(Editor::generateSceneGridLines(parameters)),
                 kExpectedMessage);
}

TEST(SceneGridGeometryTests, RejectsNonFiniteWorldExtent) {
    const Editor::SceneGridParameters parameters{
        .half_extent_cells = 512U,
        .grid_spacing = std::numeric_limits<float>::max(),
    };

    ASSERT_DEATH(static_cast<void>(Editor::generateSceneGridLines(parameters)),
                 "Scene grid world-space extent must be finite");
}
