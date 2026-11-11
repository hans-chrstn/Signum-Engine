#include "window.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/platform/event/platform_event.hpp"
#include "engine/platform/event/platform_event_queue.hpp"
#include "engine/platform/input/cursor_position.hpp"
#include "engine/platform/input/key.hpp"
#include "engine/platform/input/mouse_button.hpp"
#include "error/glfw_error.hpp"
#include <GLFW/glfw3.h>
#include <exception>
#include <optional>
#include <utility>
#include <vector>

namespace SNE::Engine::Platform {
    namespace {
        [[nodiscard]] auto tryTranslateGlfwKey(int glfw_key) noexcept
            -> std::optional<Key> {
            switch (glfw_key) {
            case GLFW_KEY_W:
                return Key::W;
            case GLFW_KEY_S:
                return Key::S;
            case GLFW_KEY_A:
                return Key::A;
            case GLFW_KEY_D:
                return Key::D;
            case GLFW_KEY_Q:
                return Key::Q;
            case GLFW_KEY_E:
                return Key::E;
            case GLFW_KEY_LEFT_SHIFT:
                return Key::LeftShift;
            case GLFW_KEY_TAB:
                return Key::Tab;
            default:
                return std::nullopt;
            }
        }

        [[nodiscard]] auto tryTranslateGlfwKeyAction(int action) noexcept
            -> std::optional<KeyAction> {
            switch (action) {
            case GLFW_PRESS:
                return KeyAction::Press;
            case GLFW_RELEASE:
                return KeyAction::Release;
            case GLFW_REPEAT:
                return KeyAction::Repeat;
            default:
                return std::nullopt;
            }
        }

        [[nodiscard]] auto tryTranslateGlfwMouseButton(int button) noexcept
            -> std::optional<MouseButton> {
            switch (button) {
            case GLFW_MOUSE_BUTTON_LEFT:
                return MouseButton::Left;
            case GLFW_MOUSE_BUTTON_RIGHT:
                return MouseButton::Right;
            case GLFW_MOUSE_BUTTON_MIDDLE:
                return MouseButton::Middle;
            default:
                return std::nullopt;
            }
        }

        [[nodiscard]] auto
        tryTranslateGlfwMouseButtonAction(int action) noexcept
            -> std::optional<MouseButtonAction> {
            switch (action) {
            case GLFW_PRESS:
                return MouseButtonAction::Press;
            case GLFW_RELEASE:
                return MouseButtonAction::Release;
            default:
                return std::nullopt;
            }
        }
    } // namespace

    Window::Window(WindowSize size, std::string title)
        : m_Size(size), m_Title(std::move(title)) {

        // Signum renders through Vulkan, so GLFW must not create an OpenGL
        // or OpenGL ES context for this window.
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

        glfwSetWindowUserPointer(m_Window, this);

        glfwSetScrollCallback(m_Window, Window::scrollCallback);

        glfwSetKeyCallback(m_Window, Window::keyCallback);

        glfwSetMouseButtonCallback(m_Window, Window::mouseButtonCallback);

        glfwSetCursorPosCallback(m_Window, Window::cursorPositionCallback);
    }

    Window::~Window() {
        glfwDestroyWindow(m_Window);
    }

    auto Window::scrollCallback(GLFWwindow *window, double x_offset,
                                double y_offset) noexcept -> void {
        auto *window_ptr =
            static_cast<Window *>(glfwGetWindowUserPointer(window));

        if (window_ptr == nullptr) {
            return;
        }

        try {
            const ScrollEvent scroll_event{
                .x_offset = x_offset,
                .y_offset = y_offset,
            };

            window_ptr->m_EventQueue.push(scroll_event);
        } catch (...) {
            if (window_ptr->m_CallbackException == nullptr) {
                window_ptr->m_CallbackException = std::current_exception();
            }
            return;
        }
    }

    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    auto Window::keyCallback(GLFWwindow *window, int key,
                             [[maybe_unused]] int scancode, int action,
                             [[maybe_unused]] int modifiers) noexcept -> void {
        auto *window_ptr =
            static_cast<Window *>(glfwGetWindowUserPointer(window));

        if (window_ptr == nullptr) {
            return;
        }

        std::optional<Key> key_used = tryTranslateGlfwKey(key);
        if (!key_used.has_value()) {
            return;
        }

        std::optional<KeyAction> action_used =
            tryTranslateGlfwKeyAction(action);
        if (!action_used.has_value()) {
            return;
        }

        try {
            const KeyEvent event{
                .key = key_used.value(),
                .action = action_used.value(),
            };

            window_ptr->m_EventQueue.push(event);
        } catch (...) {
            if (window_ptr->m_CallbackException == nullptr) {
                window_ptr->m_CallbackException = std::current_exception();
            }
            return;
        }
    }

    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    auto Window::mouseButtonCallback(GLFWwindow *window, int button, int action,
                                     [[maybe_unused]] int modifiers) noexcept
        -> void {
        auto *window_ptr =
            static_cast<Window *>(glfwGetWindowUserPointer(window));

        if (window_ptr == nullptr) {
            return;
        }

        std::optional<MouseButton> mouse_button =
            tryTranslateGlfwMouseButton(button);
        if (!mouse_button.has_value()) {
            return;
        }

        std::optional<MouseButtonAction> mouse_button_action =
            tryTranslateGlfwMouseButtonAction(action);
        if (!mouse_button_action.has_value()) {
            return;
        }

        try {
            const MouseButtonEvent event{
                .button = mouse_button.value(),
                .action = mouse_button_action.value(),
            };

            window_ptr->m_EventQueue.push(event);
        } catch (...) {
            if (window_ptr->m_CallbackException == nullptr) {
                window_ptr->m_CallbackException = std::current_exception();
            }

            return;
        }
    }

    auto Window::cursorPositionCallback(GLFWwindow *window, double x_position,
                                        double y_position) noexcept -> void {
        auto *window_ptr =
            static_cast<Window *>(glfwGetWindowUserPointer(window));

        if (window_ptr == nullptr) {
            return;
        }
        try {
            const CursorMovedEvent event{
                .position =
                    CursorPosition{
                        .x = x_position,
                        .y = y_position,
                    },
            };

            window_ptr->m_EventQueue.push(event);
        } catch (...) {
            if (window_ptr->m_CallbackException == nullptr) {
                window_ptr->m_CallbackException = std::current_exception();
            }

            return;
        }
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

    auto Window::isKeyPressed(Key key) const -> bool {
        switch (key) {
        case Key::W:
            return glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS;
        case Key::S:
            return glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS;
        case Key::A:
            return glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS;
        case Key::D:
            return glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS;
        case Key::Q:
            return glfwGetKey(m_Window, GLFW_KEY_Q) == GLFW_PRESS;
        case Key::E:
            return glfwGetKey(m_Window, GLFW_KEY_E) == GLFW_PRESS;
        case Key::LeftShift:
            return glfwGetKey(m_Window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
        case Key::Tab:
            return glfwGetKey(m_Window, GLFW_KEY_TAB) == GLFW_PRESS;
        }

        Core::Assertion::failAssertion(
            Core::Assertion::AssertionType::Precondition,
            Core::Error::Subsystem::Platform,
            "Window key query requires a supported Key "
            "value");
    }

    auto Window::setCursorCaptured(bool captured) -> void {
        glfwSetInputMode(m_Window, GLFW_CURSOR,
                         captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }

    auto Window::requestClose() -> void {
        glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
    }

    auto Window::takeEvents() -> std::vector<PlatformEvent> {
        std::exception_ptr callback_exception =
            std::exchange(m_CallbackException, nullptr);

        if (callback_exception != nullptr) {
            std::rethrow_exception(callback_exception);
        }

        return m_EventQueue.takeAll();
    }
} // namespace SNE::Engine::Platform
