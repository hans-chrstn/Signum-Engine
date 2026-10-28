#pragma once

#include "engine/core/math/matrix4.hpp"
#include "engine/renderer/geometry/initial_mesh_data.hpp"
#include "engine/renderer/geometry/primitive_mesh.hpp"
#include <span>

namespace SNE::Engine::Renderer {
    /**
     * @brief Creates an initial mesh description for primitive geometry.
     *
     * Combines CPU-owned primitive geometry, its shared vertex layout,
     * and model transformations into the renderer's initial mesh format.
     *
     * Uses the standard PrimitiveVertex attribute layout and creates
     * non-owning views of the supplied geometry and model matrices.
     *
     * The operation does not allocate memory, copy geometry, or create
     * GPU resources.
     *
     * @param mesh Primitive geometry to describe.
     * @param model_matrices Model transformations for mesh instances.
     *
     * @return Initial mesh description borrowing the supplied data.
     *
     * @warning The mesh and model matrix storage must remain valid
     * and must not be invalidated until renderer initialization has
     * consumed the returned description.
     *
     * @note An empty model matrix span requests no mesh instances.
     *
     * @note This helper applies only to the shared primitive vertex
     * format. Other mesh formats may use InitialMeshData directly.
     */
    [[nodiscard]] auto makePrimitiveInitialMeshData(
        const PrimitiveMesh &mesh,
        std::span<const Core::Math::Matrix4f> model_matrices)
        -> InitialMeshData;
} // namespace SNE::Engine::Renderer
