#pragma once

namespace SNE::Editor::Panels {
    /**
     * @brief Draws the editor's Inspector panel.
     *
     * Provides the window used to inspect and edit properties of
     * selected scene entities once scene inspection is available.
     *
     * The panel is drawn only when visible is true. Closing the
     * panel updates the supplied visibility state.
     *
     * @param visible Reference to the panel's visibility state.
     * Updated when the user closes the panel.
     */
    auto drawInspectorPanel(bool &visible) -> void;
} // namespace SNE::Editor::Panels
