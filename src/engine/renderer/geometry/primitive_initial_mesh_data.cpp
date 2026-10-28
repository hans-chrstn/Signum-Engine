#include "primitive_initial_mesh_data.hpp"
#include "engine/renderer/geometry/primitive_mesh_upload.hpp"
#include "engine/renderer/geometry/primitive_vertex_layout.hpp"

namespace SNE::Engine::Renderer {
    auto makePrimitiveInitialMeshData(
        const PrimitiveMesh &mesh,
        std::span<const Core::Math::Matrix4f> model_matrices)
        -> InitialMeshData {

        return InitialMeshData{
            .mesh = makePrimitiveMeshUploadData(mesh),
            .layout =
                VertexLayoutDescription{
                    .attributes = kPrimitiveVertexAttributes,
                },
            .model_matrices = model_matrices,
        };
    }
} // namespace SNE::Engine::Renderer
