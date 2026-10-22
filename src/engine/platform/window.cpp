#include "window.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/platform/input/cursor_position.hpp"
#include "engine/platform/input/mouse_button.hpp"
#include "error/glfw_error.hpp"
#include <GLFW/glfw3.h>
#include <utility>

namespace SNE::Engine::Platform {
    Window::Window(WindowSize size, std::string title)
        : m_Size(size), m_Title(std::move(title)) {

        // Signum renders through Vulkan, so GLFW must not create an OpenGL or
        // OpenGL ES context for this window.
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
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

    auto Window::nativeHandle() const -> GLFWwindow * {
        return m_Window;
    }

    auto Window::framebufferSize() const -> FramebufferSize {
        int width{};
        int height{};

        glfwGetFramebufferSize(m_Window, &width, &height);

        return FramebufferSize{
            .width = width,
            .height = height,
        };
    }

    auto Window::pollEvents() -> void {
        glfwPollEvents();
    }

    auto Window::cursorPosition() const -> CursorPosition {
        double x_pos{};
        double y_pos{};
        glfwGetCursorPos(m_Window, &x_pos, &y_pos);

        return CursorPosition{
            .x = x_pos,
            .y = y_pos,
        };
    }

    auto Window::isMouseButtonPressed(MouseButton button) const -> bool {
        switch (button) {
        case MouseButton::Left:
            return glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_LEFT) ==
                   GLFW_PRESS;
        case MouseButton::Right:
            return glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_RIGHT) ==
                   GLFW_PRESS;
        case MouseButton::Middle:
            return glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_MIDDLE) ==
                   GLFW_PRESS;
        }

        Core::Assertion::failAssertion(
            Core::Assertion::AssertionType::Precondition,
            Core::Error::Subsystem::Platform,
            "Window mouse-button query requires a supported MouseButton "
            "value");
    }
} // namespace SNE::Engine::Platform
