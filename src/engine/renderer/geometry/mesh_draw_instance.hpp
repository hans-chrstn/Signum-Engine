#pragma once

#include "engine/core/math/matrix4.hpp"
#include "engine/renderer/resource/gpu_resource_handle.hpp"
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
    };
} // namespace SNE::Engine::Renderer
