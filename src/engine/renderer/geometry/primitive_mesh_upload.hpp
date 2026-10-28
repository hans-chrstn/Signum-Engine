#pragma once

#include "engine/renderer/geometry/mesh_upload_data.hpp"
#include "engine/renderer/geometry/primitive_mesh.hpp"

namespace SNE::Engine::Renderer {
    /**
     * @brief Creates a non-owning upload description for primitive geometry.
     *
     * Exposes the primitive's contiguous vertex storage as bytes and
     * borrows its index storage without copying or allocating memory.
     *
     * The vertex stride is derived from PrimitiveVertex.
     *
     * @param mesh Primitive mesh whose storage will be borrowed.
     *
     * @return Mesh upload description referencing the supplied geometry.
     *
     * @warning The primitive mesh must remain alive, and its vertex and
     * index storage must not be invalidated, until the upload completes.
     *
     * @note Does not create GPU resources or perform an upload.
     */
    [[nodiscard]] auto makePrimitiveMeshUploadData(const PrimitiveMesh &mesh)
        -> MeshUploadData;
} // namespace SNE::Engine::Renderer
