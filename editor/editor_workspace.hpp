#pragma once

namespace SNE::Editor {
    /**
     * @brief Coordinates the editor's main workspace UI.
     *
     * Owns panel visibility state and draws the main menu bar,
     * dockspace, and editor panels.
     *
     * Does not own the ImGui context or rendering backend.
     */
    class EditorWorkspace {
      private:
        /**
         * @brief Whether the editor status panel is visible.
         */
        bool m_ShowEditorStatus = true;

        /**
         * @brief Whether the About window is visible.
         */
        bool m_ShowAbout = false;

        /**
         * @brief Whether the Hierarchy panel is visible.
         */
        bool m_ShowHierarchy = true;

        /**
         * @brief Whether the Inspector panel is visible.
         */
        bool m_ShowInspector = true;

        /**
         * @brief Whether the Scene View panel is visible.
         */
        bool m_ShowSceneView = true;

        /**
         * @brief Whether the Content Browser panel is visible.
         */
        bool m_ShowContentBrowserView = true;

        /**
         * @brief Whether the Performance panel is visible.
         */
        bool m_ShowPerformanceView = true;

      public:
        /**
         * @brief Draws the editor workspace for the current ImGui frame.
         *
         * Draws the main menu bar, dockspace, and editor panels.
         *
         * Must be called between ImGui::NewFrame() and ImGui::Render().
         */
        auto draw() -> void;
    };
} // namespace SNE::Editor
