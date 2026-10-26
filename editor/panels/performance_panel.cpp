#include "performance_panel.hpp"
#include <imgui.h>

namespace SNE::Editor::Panels {
    auto drawPerformancePanel(bool &visible) -> void {
        if (!visible) {
            return;
        }

        const bool begin = ImGui::Begin("Performance", &visible);
        if (begin) {
            ImGui::TextUnformatted("Performance View Unavailable.");
        }

        ImGui::End();
    }
} // namespace SNE::Editor::Panels
