#include "scene_view_panel.hpp"
#include <imgui.h>

namespace SNE::Editor::Panels {
    auto drawSceneViewPanel(bool &visible,
                            const std::function<std::optional<ImTextureID>(
                                const SceneViewPanelState &)> &resolve_texture)
        -> SceneViewPanelState {
        SceneViewPanelState panel_state{};
        if (!visible) {
            return panel_state;
        }

        const bool begin = ImGui::Begin("Scene View", &visible);

        std::optional<ImTextureID> texture;

        if (begin) {
            const ImVec2 available_region = ImGui::GetContentRegionAvail();

            panel_state.m_ContentWidth = available_region.x;
            panel_state.m_ContentHeight = available_region.y;
            panel_state.m_IsFocused = ImGui::IsWindowFocused();
            panel_state.m_IsHovered = ImGui::IsWindowHovered();

            if (available_region.x > 0.0F && available_region.y > 0.0F) {
                panel_state.m_IsVisible = true;

                if (resolve_texture) {
                    texture = resolve_texture(panel_state);
                }
            }

            if (texture.has_value()) {
                ImGui::Image(texture.value(), available_region);
                panel_state.m_IsImageHovered = ImGui::IsItemHovered();
            } else {
                ImGui::TextUnformatted("Scene viewport unavailable.");
            }
        }

        ImGui::End();

        return panel_state;
    }
} // namespace SNE::Editor::Panels
