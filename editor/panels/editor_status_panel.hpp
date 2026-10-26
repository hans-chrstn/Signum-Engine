#pragma once

namespace SNE::Editor::Panels {
    /**
     * @brief Draws the editor status panel.
     *
     * Displays basic editor status information using Dear ImGui.
     *
     * The panel is drawn only when visible is true. Closing the panel
     * through its window controls updates the supplied visibility state.
     *
     * @param visible Reference to the panel's visibility state.
     * Updated when the user closes the panel.
     */
    auto drawEditorStatusPanel(bool &visible) -> void;
} // namespace SNE::Editor::Panels
