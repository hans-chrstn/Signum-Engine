#pragma once

namespace SNE::Editor::Panels {
    /**
     * @brief Draws the editor's Content Browser panel.
     *
     * Provides the window used to browse project assets and
     * resources once asset browsing is available.
     *
     * The panel is drawn only when visible is true. Closing the
     * panel updates the supplied visibility state.
     *
     * @param visible Reference to the panel's visibility state.
     * Updated when the user closes the panel.
     */
    auto drawContentBrowserPanel(bool &visible) -> void;
} // namespace SNE::Editor::Panels
