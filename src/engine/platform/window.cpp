#include "window.hpp"
#include "engine/core/error/engine_error.hpp"
#include <GLFW/glfw3.h>

namespace SNE::Engine::Platform {
    Window::Window(int width, int height, const std::string &title)
        : m_Width(width), m_Height(height), m_Title(title) {

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);

        m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr,
                                    nullptr);
        if (m_Window == nullptr) {
            throw Core::Error::EngineError(
                Core::Error::Code::WindowCreationFailed,
                "Failed to create a window!");
        }
    }

    Window::~Window() {
        glfwDestroyWindow(m_Window);
    }

    bool Window::shouldClose() const {
        return glfwWindowShouldClose(m_Window);
    }

    void Window::waitEvents() const {
        glfwWaitEvents();
    }
} // namespace SNE::Engine::Platform
