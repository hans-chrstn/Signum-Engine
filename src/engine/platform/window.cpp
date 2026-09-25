#include "window.hpp"
#include "engine/core/error/engine_error.hpp"
#include "error/glfw_error.hpp"
#include <GLFW/glfw3.h>
#include <string>

namespace SNE::Engine::Platform {
    Window::Window(WindowSize size, std::string title)
        : m_Size(size), m_Title(std::move(title)) {

        // Signum renders through Vulkan, so GLFW must not create an OpenGL or
        // OpenGL ES
        // context for this window.
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);

        m_Window = glfwCreateWindow(m_Size.width, m_Size.height,
                                    m_Title.c_str(), nullptr, nullptr);
        if (m_Window == nullptr) {
            throw Core::Error::EngineError(
                Core::Error::Code::WindowCreationFailed,
                "Failed to create a window!", Error::captureGlfwError(),
                "Create Window");
        }
    }

    Window::~Window() {
        glfwDestroyWindow(m_Window);
    }

    auto Window::shouldClose() const -> bool {
        return glfwWindowShouldClose(m_Window) != GLFW_FALSE;
    }

    void Window::waitEvents() {
        glfwWaitEvents();
    }
} // namespace SNE::Engine::Platform
