#include "engine/core/application.hpp"
#include "engine/core/error/error_boundary.hpp"
#include "imgui/editor_imgui_context.hpp"
#include "imgui/editor_imgui_vulkan_backend.hpp"
#include <iostream>

auto main() -> int {
    return SNE::Engine::Core::Error::runWithErrorBoundary(
        [] -> void {
            SNE::Engine::Core::Application application;
            SNE::Editor::EditorImGuiContext editor_context{
                application.window().nativeHandle()};
            SNE::Editor::EditorImGuiVulkanBackend editor_backend{
                application.externalRenderingInfo()};
            application.run();
        },
        std::cerr);
}
