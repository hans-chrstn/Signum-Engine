#pragma once

#include "engine/renderer/geometry/mesh_upload_data.hpp"
#include "scene_view/scene_grid_mesh.hpp"

namespace SNE::Editor {
    /**
     * @brief Creates a non-owning mesh upload description for editor grid
     * geometry.
     *
     * Exposes the grid's contiguous vertex storage as bytes and borrows
     * its 16-bit index storage without copying or allocating memory.
     *
     * The vertex stride is derived from SceneGridVertex.
     * The resulting description can be consumed by the engine's existing
     * mesh upload infrastructure.
     *
     * @param mesh CPU-owned grid mesh whose storage will be borrowed.
     *
     * @return Mesh upload description referencing the supplied grid geometry.
     *
     * @warning The grid mesh must remain alive, and its vertex and index
     * storage must not be invalidated, until the upload completes.
     *
     * @note This function does not allocate GPU resources or perform an upload.
     * @note Primitive topology and vertex attribute layout are configured
     * separately.
     */
    [[nodiscard]] auto makeSceneGridMeshUploadData(const SceneGridMesh &mesh)
        -> Engine::Renderer::MeshUploadData;
} // namespace SNE::Editor
