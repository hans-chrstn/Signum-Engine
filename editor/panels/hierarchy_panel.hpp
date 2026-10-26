#pragma once

namespace SNE::Editor::Panels {
    /**
     * @brief Draws the editor's Hierarchy panel.
     *
     * Provides the window used to display scene entities and their
     * relationships once scene inspection is available.
     *
     * The panel is drawn only when visible is true. Closing the
     * panel updates the supplied visibility state.
     *
     * @param visible Reference to the panel's visibility state.
     * Updated when the user closes the panel.
     */
    auto drawHierarchyPanel(bool &visible) -> void;
} // namespace SNE::Editor::Panels
