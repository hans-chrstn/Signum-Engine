#include "inspector_panel.hpp"
#include <imgui.h>

namespace SNE::Editor::Panels {
    auto drawInspectorPanel(bool &visible) -> void {
        if (!visible) {
            return;
        }

        const bool begin = ImGui::Begin("Inspector", &visible);
        if (begin) {
            ImGui::TextUnformatted("No entity selected");
        }

        ImGui::End();
    }
} // namespace SNE::Editor::Panels
