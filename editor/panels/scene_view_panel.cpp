#include "scene_view_panel.hpp"
#include <imgui.h>

namespace SNE::Editor::Panels {
    auto drawSceneViewPanel(bool &visible) -> void {
        if (!visible) {
            return;
        }

        const bool begin = ImGui::Begin("Scene View", &visible);
        if (begin) {
            ImGui::TextUnformatted("Scene viewport unavailable.");
        }

        ImGui::End();
    }
} // namespace SNE::Editor::Panels
