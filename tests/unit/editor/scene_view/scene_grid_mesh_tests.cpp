#include "scene_view/scene_grid_mesh.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>
#include <span>
#include <vector>

namespace Editor = SNE::Editor;

TEST(SceneGridMeshTests, ConvertsLinesIntoIndexedVertices) {
    const std::array<Editor::SceneGridLine, 2U> lines{
        {
            {
                .start = {.x = -2.0F, .y = 0.0F, .z = 1.5F},
                .end = {.x = 2.0F, .y = 0.0F, .z = 1.5F},
            },
            {
                .start = {.x = 0.0F, .y = 0.0F, .z = -3.0F},
                .end = {.x = 0.0F, .y = 0.0F, .z = 3.0F},
            },
        },
    };

    const Editor::SceneGridMesh mesh = Editor::makeSceneGridMesh(lines);

    ASSERT_EQ(mesh.vertices.size(), 4U);
    ASSERT_EQ(mesh.indices.size(), 4U);

    EXPECT_EQ(mesh.vertices[0].position,
              (std::array<float, 3U>{-2.0F, 0.0F, 1.5F}));
    EXPECT_EQ(mesh.vertices[1].position,
              (std::array<float, 3U>{2.0F, 0.0F, 1.5F}));
    EXPECT_EQ(mesh.vertices[2].position,
              (std::array<float, 3U>{0.0F, 0.0F, -3.0F}));
    EXPECT_EQ(mesh.vertices[3].position,
              (std::array<float, 3U>{0.0F, 0.0F, 3.0F}));

    EXPECT_EQ(mesh.indices, (std::vector<std::uint16_t>{0U, 1U, 2U, 3U}));
}

TEST(SceneGridMeshTests, AcceptsEmptyInput) {
    const Editor::SceneGridMesh mesh =
        Editor::makeSceneGridMesh(std::span<const Editor::SceneGridLine>{});

    EXPECT_TRUE(mesh.vertices.empty());
    EXPECT_TRUE(mesh.indices.empty());
}

TEST(SceneGridMeshTests, AcceptsMaximum16BitIndexCapacity) {
    constexpr std::size_t kMaximumLines{32768U};

    const std::vector<Editor::SceneGridLine> lines(kMaximumLines);

    const Editor::SceneGridMesh mesh = Editor::makeSceneGridMesh(lines);

    ASSERT_EQ(mesh.vertices.size(), 65536U);
    ASSERT_EQ(mesh.indices.size(), 65536U);

    EXPECT_EQ(mesh.indices.front(), 0U);
    EXPECT_EQ(mesh.indices.back(), 65535U);
}

TEST(SceneGridMeshTests, RejectsExcessiveLineCount) {
    constexpr std::size_t kExcessiveLines{32769U};

    const std::vector<Editor::SceneGridLine> lines(kExcessiveLines);

    ASSERT_DEATH(static_cast<void>(Editor::makeSceneGridMesh(lines)),
                 "Scene grid mesh line count exceeds 16-bit index capacity");
}
