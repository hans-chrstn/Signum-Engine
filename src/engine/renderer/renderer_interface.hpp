#pragma once

#include "engine/renderer/graphics_backend.hpp"
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
    };
} // namespace SNE::Engine::Renderer
