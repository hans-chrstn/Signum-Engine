#pragma once

#include "engine/renderer/geometry/mesh_draw_instance.hpp"
#include "engine/renderer/graphics_backend.hpp"
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
    };
} // namespace SNE::Engine::Renderer
