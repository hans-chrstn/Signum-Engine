#include "editor_workspace.hpp"
#include "panels/about_window.hpp"
#include "panels/editor_status_panel.hpp"
#include <imgui.h>

namespace SNE::Editor {
    auto EditorWorkspace::draw() -> void {
        const bool main_menu = ImGui::BeginMainMenuBar();
        if (main_menu) {
            const bool view_menu = ImGui::BeginMenu("View");
            if (view_menu) {
                ImGui::MenuItem("Editor Status", nullptr, &m_ShowEditorStatus);
                ImGui::EndMenu();
            }

            const bool help_menu = ImGui::BeginMenu("Help");
            if (help_menu) {
                if (ImGui::MenuItem("About")) {
                    m_ShowAbout = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        ImGui::DockSpaceOverViewport();
        Panels::drawEditorStatusPanel(m_ShowEditorStatus);
        Panels::drawAboutWindow(m_ShowAbout);
    }
} // namespace SNE::Editor
