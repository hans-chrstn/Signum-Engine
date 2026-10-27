#include "scene_view_panel.hpp"
#include <imgui.h>

namespace SNE::Editor::Panels {
    auto drawSceneViewPanel(bool &visible) -> SceneViewPanelState {
        SceneViewPanelState panel_state{};
        if (!visible) {
            return panel_state;
        }

        const bool begin = ImGui::Begin("Scene View", &visible);
        if (begin) {
            const ImVec2 available_region = ImGui::GetContentRegionAvail();

            panel_state.m_ContentWidth = available_region.x;
            panel_state.m_ContentHeight = available_region.y;
            panel_state.m_IsFocused = ImGui::IsWindowFocused();
            panel_state.m_IsHovered = ImGui::IsWindowHovered();

            if (available_region.x > 0.0F && available_region.y > 0.0F) {
                panel_state.m_IsVisible = true;
            }

            ImGui::TextUnformatted("Scene viewport unavailable.");
        }

        ImGui::End();

        return panel_state;
    }
} // namespace SNE::Editor::Panels
