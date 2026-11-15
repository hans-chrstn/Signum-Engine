#pragma once

#include "engine/renderer/renderer_interface.hpp"
#include <memory>

namespace SNE::Engine::Renderer {
    struct RendererCreationInfo;

    /**
     * @brief Creates a renderer for the requested graphics backend.
     *
     * Selects the concrete renderer implementation using the supplied
     * backend-independent initialization information.
     *
     * Transfers ownership of created rendering resources to the caller
     * through the IRenderer interface.
     *
     * @param creation_info Renderer initialization configuration and
     * resources required to construct the selected implementation.
     *
     * @return Owning pointer to the initialized renderer.
     *
     * @pre Borrowed initialization resources must remain valid until
     * renderer construction completes.
     *
     * @throws Core::Error::EngineError if the selected backend cannot be
     * initialized or a required rendering resource cannot be created.
     *
     * @note The factory does not own the application window.
     * The window must outlive the returned renderer.
     *
     * @note Unsupported backend selections must not silently fall back
     * to another graphics API.
     */
    [[nodiscard]] auto createRenderer(RendererCreationInfo creation_info)
        -> std::unique_ptr<IRenderer>;
} // namespace SNE::Engine::Renderer
