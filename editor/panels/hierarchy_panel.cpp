#include "hierarchy_panel.hpp"
#include <imgui.h>

namespace SNE::Editor::Panels {
    auto drawHierarchyPanel(bool &visible) -> void {
        if (!visible) {
            return;
        }

        const bool begin = ImGui::Begin("Hierarchy", &visible);
        if (begin) {
            ImGui::TextUnformatted("No scene loaded.");
        }

        ImGui::End();
    }
} // namespace SNE::Editor::Panels
