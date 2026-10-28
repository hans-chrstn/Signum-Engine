#pragma once

#include "panels/scene_view_panel.hpp"
#include <functional>
#include <imgui.h>
#include <optional>
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

        /**
         * @brief Stores the Scene View panel state from the current editor
         * frame.
         *
         * Contains the drawable content dimensions, focus state, hover state,
         * and visibility reported by the Scene View panel.
         *
         * Updated when the editor workspace draws its panels.
         *
         * Does not own scene rendering resources.
         */
        Panels::SceneViewPanelState m_SceneViewPanelState{};

      public:
        /**
         * @brief Draws the editor workspace for the current ImGui frame.
         *
         * Draws the main menu bar, dockspace, and editor panels.
         *
         * Forwards the optional scene texture resolver to the Scene View
         * panel, allowing it to display a renderer-provided texture
         * without managing Vulkan resources.
         *
         * Must be called between ImGui::NewFrame() and ImGui::Render().
         *
         * @param resolve_scene_texture Optional callback used to obtain
         * an ImGui texture identifier for the current Scene View content
         * area.
         *
         * The callback receives the Scene View panel state and returns
         * std::nullopt when no compatible texture is available.
         *
         * @return true if the user requests application exit during the
         * current frame; otherwise false.
         *
         * @note The workspace does not own scene textures or Vulkan
         * descriptor registrations.
         */
        [[nodiscard]] auto draw(const std::function<std::optional<ImTextureID>(
                                    const Panels::SceneViewPanelState &)>
                                    &resolve_scene_texture = {}) -> bool;

        /**
         * @brief Returns the latest Scene View panel state.
         *
         * Provides the content dimensions, visibility, and input state
         * recorded during the most recent workspace draw.
         *
         * The returned value is a snapshot and does not transfer ownership
         * of rendering resources.
         *
         * @return Copy of the latest Scene View panel state.
         *
         * @note Before the first workspace draw, the returned state
         * is default-initialized.
         */
        [[nodiscard]] auto sceneViewPanelState() const noexcept
            -> Panels::SceneViewPanelState;
    };
} // namespace SNE::Editor
