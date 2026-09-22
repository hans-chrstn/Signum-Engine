#include "application.hpp"
#include "engine/platform/window.hpp"

namespace SNE::Engine::Core {
    void Application::run() {
        Platform::Window window(600, 400, "Engine");

        while (!window.shouldClose()) {
            window.waitEvents();
        }
    }
} // namespace SNE::Engine::Core
