#include "application.hpp"
#include "engine/platform/window.hpp"

namespace SNE::Engine::Core {
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void Application::run() {
        constexpr Platform::WindowSize initialWindowSize{.width = 600,
                                                         .height = 400};
        Platform::Window window(initialWindowSize, "Engine");

        while (!window.shouldClose()) {
            Platform::Window::waitEvents();
        }
    }
} // namespace SNE::Engine::Core
