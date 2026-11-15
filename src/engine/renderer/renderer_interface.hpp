#pragma once

#include "engine/core/math/matrix4.hpp"
#include "engine/renderer/geometry/mesh_draw_instance.hpp"
#include "engine/renderer/geometry/mesh_upload_data.hpp"
#include "engine/renderer/geometry/vertex_layout_description.hpp"
#include "engine/renderer/graphics_backend.hpp"
#include "engine/renderer/render_target_extent.hpp"
#include "engine/renderer/resource/gpu_resource_handle.hpp"
#include <cstddef>
#include <optional>
#include <span>
#include <vector>

namespace SNE::Engine::Renderer {
    /**
     * @brief Defines the backend-independent renderer ownership interface.
     *
     * Provides a polymorphic contract through which higher-level engine
     * systems interact with rendering implementations.
     *
     * Concrete renderers own their backend-specific rendering resources
     * and implement operations exposed through this interface.
     *
     * The interface does not own a graphics device or GPU resources.
     * Resource ownership remains the responsibility of concrete
     * implementations and their associated device contexts.
     *
     * @note The renderer interface is distinct from the lower-level
     * Render Hardware Interface (RHI).
     *
     * @note Backend-specific functionality may be exposed through
     * explicitly defined extension mechanisms.
     */
    class IRenderer {
      public:
        /** @brief Constructs the renderer interface base. */
        IRenderer() = default;

        /**
         * @brief Destroys a renderer through its polymorphic interface.
         *
         * Allows an owning application to release a concrete renderer
         * through a pointer to the backend-independent interface.
         *
         * Concrete implementations remain responsible for destroying
         * their GPU resources in the required dependency order.
         */
        virtual ~IRenderer() noexcept = default;

        IRenderer(const IRenderer &) = delete;

        auto operator=(const IRenderer &) -> IRenderer & = delete;

        IRenderer(IRenderer &&other) = delete;

        auto operator=(IRenderer &&other) -> IRenderer & = delete;

        /**
         * @brief Identifies the graphics backend implemented by this renderer.
         *
         * Returns the graphics API selected when the concrete renderer
         * was created.
         *
         * @return Graphics backend implemented by this renderer.
         *
         * @note This function does not initialize, replace, or switch
         * the active rendering backend.
         */
        [[nodiscard]] virtual auto graphicsBackend() const noexcept
            -> GraphicsBackend = 0;

        /**
         * @brief Replaces the mesh draw instances managed by the renderer.
         *
         * Validates the supplied draw instances and stores an independent
         * copy for subsequent rendering operations.
         *
         * Each instance references registered mesh and optional material
         * resources through non-owning renderer resource handles.
         *
         * An empty collection removes all currently configured instances.
         *
         * @param instances Borrowed mesh draw instances to configure.
         *
         * @pre Every referenced resource must belong to this renderer
         * and remain registered.
         * @pre Updates must be serialized with rendering and resource
         * modification operations.
         *
         * @post On success, the renderer retains the supplied instances.
         * @post On allocation failure, the previous instances remain unchanged.
         *
         * @throws std::bad_alloc if storing the instances fails.
         *
         * @note This function does not transfer ownership of GPU resources.
         * @note The supplied span only needs to remain valid until the call
         * completes.
         */
        virtual auto
        setDrawInstances(std::span<const MeshDrawInstance> instances)
            -> void = 0;

        /**
         * @brief Returns a snapshot of the renderer's current mesh draw
         * instances.
         *
         * Copies the configured draw instances into an independently owned
         * collection, preserving their rendering order.
         *
         * Modifying the returned collection does not change renderer state.
         *
         * @return Owned copy of the currently configured mesh draw instances.
         *
         * @pre Access must be serialized with rendering and draw-instance
         * modification operations.
         *
         * @post The renderer's draw instances remain unchanged.
         *
         * @throws std::bad_alloc if snapshot allocation fails.
         *
         * @note Resource handles contained in the snapshot remain non-owning
         * references to renderer-managed resources.
         *
         * @note The snapshot may become outdated after subsequent renderer
         * modifications.
         */
        [[nodiscard]] virtual auto drawInstances() const
            -> std::vector<MeshDrawInstance> = 0;

        /**
         * @brief Updates the camera view transformation.
         *
         * Sets the world-to-view matrix used during subsequent rendering
         * operations while preserving the existing projection parameters.
         *
         * @param view_matrix Replacement world-to-view transformation.
         *
         * @pre An initial camera configuration must exist.
         * @pre Every matrix element must be finite.
         * @pre Updates must be serialized with frame recording.
         *
         * @post Subsequent frames use the supplied view matrix.
         *
         * @note This operation does not recreate GPU resources.
         */
        virtual auto
        setCameraViewMatrix(const Core::Math::Matrix4f &view_matrix)
            -> void = 0;

        /**
         * @brief Updates an existing draw instance's model transformation.
         *
         * Replaces the model matrix without changing the referenced mesh
         * or material resources.
         *
         * @param instance_index Zero-based draw-instance index.
         * @param model_matrix Replacement model transformation.
         *
         * @pre The index must identify an existing draw instance.
         * @pre Updates must be serialized with rendering.
         *
         * @post The selected instance uses the supplied transformation.
         * @post Other draw instances remain unchanged.
         *
         * @note Does not upload geometry or recreate GPU resources.
         */
        virtual auto
        setDrawInstanceModelMatrix(std::size_t instance_index,
                                   const Core::Math::Matrix4f &model_matrix)
            -> void = 0;

        /**
         * @brief Updates the requested offscreen scene viewport dimensions.
         *
         * Stores the framebuffer-pixel extent used for subsequent scene
         * rendering. An empty extent disables the viewport request.
         *
         * @param extent Requested viewport extent, or std::nullopt.
         *
         * @pre An engaged extent must have nonzero width and height.
         * @pre Updates must be serialized with frame rendering.
         *
         * @post The requested extent matches the supplied value.
         *
         * @note GPU target allocation and synchronization occur separately
         * during frame processing.
         */
        virtual auto
        setSceneViewportExtent(std::optional<RenderTargetExtent> extent)
            -> void = 0;

        /**
         * @brief Uploads and registers a mesh with an explicit vertex layout.
         *
         * Creates renderer-owned GPU resources from borrowed vertex and
         * index data and registers the resulting mesh for subsequent draws.
         *
         * Registration does not change the active mesh or draw instances.
         *
         * @param mesh Borrowed CPU geometry data to upload.
         * @param layout Borrowed vertex attribute layout.
         *
         * @return Handle identifying the registered mesh resource.
         *
         * @pre Mesh data and vertex layout must be valid and compatible.
         * @pre Registration must be serialized with rendering and resource
         * modification operations.
         *
         * @post On success, the returned handle resolves to the registered
         * mesh.
         * @post On failure, no new mesh remains registered.
         *
         * @throws Core::Error::EngineError if GPU allocation or upload fails.
         * @throws std::bad_alloc if host allocation or registration fails.
         *
         * @note Invalid input descriptions trigger precondition assertions.
         * @note The renderer owns GPU resources; the returned handle does not.
         * @note Borrowed CPU storage must remain valid until upload completes.
         */
        [[nodiscard]] virtual auto
        registerMesh(const MeshUploadData &mesh,
                     const VertexLayoutDescription &layout)
            -> GpuResourceHandle = 0;
    };
} // namespace SNE::Engine::Renderer
