#pragma once

#include "engine/core/math/matrix4.hpp"
#include "engine/renderer/resource/gpu_resource_handle.hpp"
#include <optional>

namespace SNE::Engine::Renderer {
    /**
     * @brief Describes one mesh instance submitted for rendering.
     *
     * Associates a registered mesh resource with the transformation used
     * to draw it.
     *
     * This description does not own GPU resources or represent a persistent
     * scene or game-object identity.
     */
    struct MeshDrawInstance {
        /** @brief Non-owning handle identifying the mesh to draw. */
        GpuResourceHandle mesh_handle;

        /** @brief Model transformation applied to this instance. */
        Core::Math::Matrix4f model_matrix;

        /**
         * @brief Optional material instance used to render this mesh.
         *
         * References a renderer-registered material without owning it.
         * An empty value selects the renderer's default material.
         */
        std::optional<GpuResourceHandle> material_handle;
    };
} // namespace SNE::Engine::Renderer
