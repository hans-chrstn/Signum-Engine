#pragma once

namespace SNE::Editor::Panels {
    /**
     * @brief Draws the editor's Performance panel.
     *
     * Provides the window used to display engine performance
     * information once performance monitoring is available.
     *
     * The panel is drawn only when visible is true. Closing the
     * panel updates the supplied visibility state.
     *
     * @param visible Reference to the panel's visibility state.
     * Updated when the user closes the panel.
     */
    auto drawPerformancePanel(bool &visible) -> void;
} // namespace SNE::Editor::Panels
