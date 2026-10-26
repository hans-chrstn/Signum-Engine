#pragma once

namespace SNE::Editor::Panels {
    /**
     * @brief Draws the editor's Scene View panel.
     *
     * Provides the dockable window used to display and interact with
     * the scene rendered by the engine.
     *
     * Scene rendering integration is managed separately.
     *
     * The panel is drawn only when visible is true. Closing the
     * panel updates the supplied visibility state.
     *
     * @param visible Reference to the panel's visibility state.
     * Updated when the user closes the panel.
     */
    auto drawSceneViewPanel(bool &visible) -> void;
} // namespace SNE::Editor::Panels
