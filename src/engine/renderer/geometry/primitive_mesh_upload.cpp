#include "primitive_mesh_upload.hpp"
#include <span>

namespace SNE::Engine::Renderer {
    auto makePrimitiveMeshUploadData(const PrimitiveMesh &mesh)
        -> MeshUploadData {
        return {
            .vertex_bytes = std::as_bytes(std::span{mesh.vertices}),
            .vertex_stride =
                static_cast<std::uint32_t>(sizeof(PrimitiveVertex)),
            .indices = mesh.indices,
        };
    }
} // namespace SNE::Engine::Renderer
