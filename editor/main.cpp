#include "engine/core/application.hpp"
#include "engine/core/error/error_boundary.hpp"
#include "imgui/editor_imgui_context.hpp"
#include <iostream>

auto main() -> int {
    return SNE::Engine::Core::Error::runWithErrorBoundary(
        [] -> void {
            SNE::Engine::Core::Application application;
            SNE::Editor::EditorImGuiContext editor_context{
                application.window().nativeHandle()};
            application.run();
        },
        std::cerr);
}
