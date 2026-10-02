#include "application.hpp"
#include <utility>

namespace SNE::Engine::Core {
    Application::Application(ApplicationConfiguration configuration)
        : m_Configuration(std::move(configuration)),
          m_Window(m_Configuration.initial_window_size,
                   m_Configuration.application_name),
          m_Renderer(m_Configuration.application_name, m_Window,
                     m_Configuration.presentation_preference,
                     m_Configuration.development_diagnostics_enabled) {}

    void Application::run() {
        while (!m_Window.shouldClose()) {
            Platform::Window::pollEvents();
            m_Renderer.renderFrame();
        }
    }
} // namespace SNE::Engine::Core
