#pragma once

namespace SNE::Editor::Panels {
    /**
     * @brief Draws the editor's About window.
     *
     * Displays basic information about Signum Engine and its editor.
     *
     * The window is drawn only when visible is true. Closing the
     * window updates the supplied visibility state.
     *
     * @param visible Reference to the About window's visibility state.
     * Updated when the user closes the window.
     */
    auto drawAboutWindow(bool &visible) -> void;
} // namespace SNE::Editor::Panels
