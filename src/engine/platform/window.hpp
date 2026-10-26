#pragma once
#include "engine/platform/input/cursor_position.hpp"
#include "engine/platform/input/key.hpp"
#include "engine/platform/input/mouse_button.hpp"
#include <GLFW/glfw3.h>
#include <string>

namespace SNE::Engine::Platform {
    /**
     * @brief Specifies the dimensions of a platform window.
     */
    struct WindowSize {
        /** Window width in logical window units. */
        int width;
        /** Window height in logical window units. */
        int height;
    };

    /**
     * @brief Specifies the dimensions of a window framebuffer in pixels.
     *
     * Framebuffer dimensions may differ from logical window dimensions on
     * high-DPI displays and may change during the lifetime of the window.
     */
    struct FramebufferSize {
        /** Framebuffer width in pixels. */
        int width;
        /** Framebuffer height in pixels. */
        int height;
    };

    /**
     * @brief Owns a GLFW window used by the engine.
     *
     * Window manages the lifetime of its underlying GLFWwindow using RAII.
     * GLFW must already be initialized and must remain initialized for the
     * lifetime of the Window.
     *
     * The window is created without a client graphics API because rendering is
     * provided through Vulkan.
     *
     * The type is non-copyable because it exclusively owns the native window.
     */
    class Window {
      private:
        WindowSize m_Size;
        std::string m_Title;
        GLFWwindow *m_Window = nullptr;

      public:
        /**
         * @brief Creates a platform window.
         *
         * @param size Initial dimensions of the window.
         * @param title Initial window title.
         *
         * @throws Core::Error::EngineError if GLFW cannot create the window.
         */
        Window(WindowSize size, std::string title);

        /**
         * @brief Destroys the owned GLFW window.
         */
        ~Window();

        Window(const Window &) = delete;
        auto operator=(const Window &) -> Window & = delete;

        /**
         * @brief Reports whether the window has been requested to close.
         *
         * @return true when GLFW indicates that the window should close;
         * otherwise false.
         */
        [[nodiscard]] auto shouldClose() const -> bool;

        /**
         * @brief Blocks until GLFW receives one or more platform events.
         */
        static void waitEvents();

        /**
         * @brief Returns the underlying GLFW window handle.
         *
         * The returned pointer is non-owning and remains valid only while this
         * Window object remains alive.
         *
         * @return Pointer to the GLFW window owned by this object.
         */
        [[nodiscard]] auto nativeHandle() const -> GLFWwindow *;

        /**
         * @brief Returns the current framebuffer dimensions of the window.
         *
         * The returned dimensions describe the drawable framebuffer size in
         * pixels and may differ from the logical window dimensions.
         *
         * @return Current framebuffer dimensions in pixels.
         */
        [[nodiscard]] auto framebufferSize() const -> FramebufferSize;

        /**
         * @brief Processes pending platform events without blocking.
         */
        static void pollEvents();

        /**
         * @brief Retrieves the current cursor position relative to the window.
         *
         * Returns the cursor's horizontal and vertical coordinates
         * in logical window units, with the origin at the upper-left
         * corner of the window's content area.
         *
         * The cursor position may fall outside the window's bounds.
         *
         * @return Current cursor position in window-relative coordinates.
         */
        [[nodiscard]] auto cursorPosition() const -> CursorPosition;

        /**
         * @brief Checks whether a mouse button is currently pressed.
         *
         * Queries the current input state of the specified mouse button.
         *
         * @param button Backend-independent mouse button to query.
         *
         * @return true if the button is pressed; otherwise false.
         *
         * @note This function queries button state rather than reporting
         * individual press or release events.
         */
        [[nodiscard]] auto isMouseButtonPressed(MouseButton button) const
            -> bool;

        /**
         * @brief Checks whether a keyboard key is currently pressed.
         *
         * Queries the current input state of the specified keyboard key.
         *
         * @param key Backend-independent keyboard key to query.
         *
         * @return true if the key is pressed; otherwise false.
         *
         * @note This function queries key state rather than reporting
         * individual press or release events.
         */
        [[nodiscard]] auto isKeyPressed(Key key) const -> bool;

        /**
         * @brief Captures or releases the cursor within the window.
         *
         * When captured, the cursor is hidden and provides virtual cursor
         * coordinates suitable for unrestricted camera rotation.
         *
         * When released, the cursor returns to normal visibility and
         * movement, allowing interaction with window UI elements.
         *
         * @param captured true to capture the cursor; false to release it.
         *
         * @note Capturing the cursor does not automatically process mouse
         * movement or update the camera orientation.
         */
        auto setCursorCaptured(bool captured) -> void;

        /**
         * @brief Requests that the window close.
         *
         * Marks the underlying GLFW window for closure.
         *
         * Does not immediately destroy the window or its resources.
         * The application event loop observes the request and exits
         * through its normal shutdown path.
         */
        auto requestClose() -> void;
    };
} // namespace SNE::Engine::Platform
