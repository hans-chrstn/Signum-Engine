#include "application.hpp"
#include "engine/platform/window.hpp"
#include <utility>

namespace SNE::Engine::Core {
    Application::Application(ApplicationConfiguration configuration)
        : m_Configuration(std::move(configuration)),
          m_Window(m_Configuration.initial_window_size,
                   m_Configuration.application_name),
          m_Renderer(m_Configuration.application_name, m_Window,
                     m_Configuration.presentation_preference,
                     m_Configuration.development_diagnostics_enabled,
                     m_Configuration.initial_mesh,
                     m_Configuration.initial_texture,
                     m_Configuration.initial_camera,
                     std::move(m_Configuration.initial_graphics_shaders)) {}

    auto Application::run(const std::function<void(Application &)> &on_update)
        -> void {
        while (!m_Window.shouldClose()) {
            Platform::Window::pollEvents();

            if (on_update) {
                on_update(*this);
            }

            m_Renderer.renderFrame();
        }
    }

    auto Application::window() const noexcept -> const Platform::Window & {
        return m_Window;
    }

    auto
    Application::setDrawInstanceModelMatrix(std::size_t instance_index,
                                            const Math::Matrix4f &model_matrix)
        -> void {
        m_Renderer.setDrawInstanceModelMatrix(instance_index, model_matrix);
    }
} // namespace SNE::Engine::Core
