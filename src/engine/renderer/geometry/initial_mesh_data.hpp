#pragma once

#include "engine/core/math/matrix4.hpp"
#include "engine/renderer/geometry/mesh_upload_data.hpp"
#include "engine/renderer/geometry/vertex_layout_description.hpp"
#include <span>

namespace SNE::Engine::Renderer {
    /**
     * @brief Describes initial mesh data and its vertex layout.
     *
     * Combines CPU-side mesh upload data with the vertex layout required
     * to interpret its vertex attributes.
     *
     * Both descriptions contain non-owning views of caller-managed
     * storage. The underlying vertex data, indices, and attribute
     * descriptions must remain valid throughout renderer initialization
     * and consumption.
     *
     * The renderer may upload the supplied mesh into GPU-owned buffers
     * and use the vertex layout to construct a compatible graphics
     * pipeline without retaining the borrowed CPU storage.
     *
     * @note The vertex layout must describe attributes compatible with
     * the mesh's vertex stride and vertex data representation.
     */
    struct InitialMeshData {
        /**
         * @brief Borrowed vertex and index data used for GPU upload.
         */
        MeshUploadData mesh{};

        /**
         * @brief Borrowed vertex attribute layout associated with the mesh.
         */
        VertexLayoutDescription layout{};

        /**
         * @brief Model transformations for drawing instances of the mesh.
         *
         * Borrows CPU-side model matrices that must remain valid until
         * renderer initialization has copied them.
         *
         * An empty span means no mesh instances are requested.
         */
        std::span<const Core::Math::Matrix4f> model_matrices;
    };
} // namespace SNE::Engine::Renderer
