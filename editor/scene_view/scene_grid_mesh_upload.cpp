#include "scene_grid_mesh_upload.hpp"
#include "engine/renderer/geometry/mesh_upload_data.hpp"
#include "scene_view/scene_grid_mesh.hpp"
#include <cstdint>
#include <span>

namespace SNE::Editor {
    auto makeSceneGridMeshUploadData(const SceneGridMesh &mesh)
        -> Engine::Renderer::MeshUploadData {
        return Engine::Renderer::MeshUploadData{
            .vertex_bytes = std::as_bytes(std::span{mesh.vertices}),
            .vertex_stride =
                static_cast<std::uint32_t>(sizeof(SceneGridVertex)),
            .indices = mesh.indices,
        };
    }
} // namespace SNE::Editor
