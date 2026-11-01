#include "scene_view/scene_grid_mesh_upload.hpp"
#include <gtest/gtest.h>
#include <span>

namespace Editor = SNE::Editor;

TEST(SceneGridMeshUploadTests, DescribesGridMeshStorage) {
    const Editor::SceneGridMesh mesh{
        .vertices =
            {
                {.position = {-1.0F, 0.0F, 0.0F}},
                {.position = {1.0F, 0.0F, 0.0F}},
            },
        .indices = {0U, 1U},
    };

    const auto upload = Editor::makeSceneGridMeshUploadData(mesh);

    EXPECT_EQ(upload.vertex_stride, sizeof(Editor::SceneGridVertex));
    EXPECT_EQ(upload.vertex_bytes.size(),
              mesh.vertices.size() * sizeof(Editor::SceneGridVertex));

    ASSERT_EQ(upload.indices.size(), 2U);
    EXPECT_EQ(upload.indices[0], 0U);
    EXPECT_EQ(upload.indices[1], 1U);
}

TEST(SceneGridMeshUploadTests, BorrowsOriginalStorage) {
    const Editor::SceneGridMesh mesh{
        .vertices =
            {
                {.position = {-2.0F, 0.0F, 0.0F}},
                {.position = {2.0F, 0.0F, 0.0F}},
            },
        .indices = {0U, 1U},
    };

    const auto upload = Editor::makeSceneGridMeshUploadData(mesh);

    EXPECT_EQ(upload.vertex_bytes.data(),
              std::as_bytes(std::span{mesh.vertices}).data());

    EXPECT_EQ(upload.indices.data(), mesh.indices.data());
}

TEST(SceneGridMeshUploadTests, AcceptsEmptyMesh) {
    const Editor::SceneGridMesh mesh{};

    const auto upload = Editor::makeSceneGridMeshUploadData(mesh);

    EXPECT_TRUE(upload.vertex_bytes.empty());
    EXPECT_TRUE(upload.indices.empty());
    EXPECT_EQ(upload.vertex_stride, sizeof(Editor::SceneGridVertex));
}
