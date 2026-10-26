#include "content_browser_panel.hpp"
#include <imgui.h>

namespace SNE::Editor::Panels {
    auto drawContentBrowserPanel(bool &visible) -> void {
        if (!visible) {
            return;
        }

        const bool begin = ImGui::Begin("Content Browser", &visible);
        if (begin) {
            ImGui::TextUnformatted("Content Browser View Unavailable.");
        }

        ImGui::End();
    }
} // namespace SNE::Editor::Panels
