#include "application.hpp"

namespace {
    constexpr SNE::Engine::Platform::WindowSize kInitialWindowSize{
        .width = 600,
        .height = 400,
    };
}

namespace SNE::Engine::Core {
    Application::Application()
        : m_Window(kInitialWindowSize, "Signum Editor"),
          m_VulkanSurface(m_VulkanInstance.nativeHandle(),
                          m_Window.nativeHandle()) {}

    void Application::run() {
        while (!m_Window.shouldClose()) {
            Platform::Window::waitEvents();
        }
    }
} // namespace SNE::Engine::Core
