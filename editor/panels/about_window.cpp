#include "about_window.hpp"
#include <imgui.h>

namespace SNE::Editor::Panels {
    auto drawAboutWindow(bool &visible) -> void {
        if (!visible) {
            return;
        }

        bool begin = ImGui::Begin("About Signum", &visible);
        if (begin) {
            ImGui::TextUnformatted("Signum Engine");
            ImGui::TextUnformatted("Signum Editor");
            ImGui::TextUnformatted("C++ 26 / Vulkan");
        }
        ImGui::End();
    }
} // namespace SNE::Editor::Panels
