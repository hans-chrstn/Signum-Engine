#include "editor_status_panel.hpp"
#include <imgui.h>

namespace SNE::Editor::Panels {
    auto drawEditorStatusPanel(bool &visible) -> void {
        if (!visible) {
            return;
        }

        ImGui::Begin("Editor Status", &visible);
        ImGui::TextUnformatted("Signum Editor");
        ImGui::End();
    }
} // namespace SNE::Editor::Panels
