#pragma once
#include "engine/platform/event/platform_event.hpp"
#include "engine/platform/event/platform_event_queue.hpp"
#include "engine/platform/input/cursor_position.hpp"
#include "engine/platform/input/key.hpp"
#include "engine/platform/input/mouse_button.hpp"
#include <GLFW/glfw3.h>
#include <exception>
#include <string>
#include <vector>

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
        /**
         * @brief Initial logical dimensions requested for the window.
         *
         * Stores the dimensions supplied during window construction.
         *
         * These values do not automatically reflect subsequent window
         * resizing. Current dimensions must be queried separately.
         */
        WindowSize m_Size;

        /**
         * @brief Title assigned to the platform window.
         *
         * Owns the title string supplied during construction and
         * used when creating the native GLFW window.
         */
        std::string m_Title;

        /**
         * @brief Owned native GLFW window handle.
         *
         * Created during window construction and destroyed by the
         * Window destructor.
         *
         * The handle remains valid throughout the window's lifetime
         * and must not be destroyed by external callers.
         *
         * GLFW must remain initialized while this handle exists.
         */
        GLFWwindow *m_Window = nullptr;

        /**
         * @brief Stores pending platform events received by this window.
         *
         * Owns the backend-independent events translated from native
         * GLFW callbacks.
         *
         * Events remain queued in arrival order until retrieved
         * through takeEvents().
         *
         * Each Window maintains its own independent event queue.
         */
        PlatformEventQueue m_EventQueue{};

        /**
         * @brief Retains the first exception raised during event callbacks.
         *
         * GLFW callbacks must not propagate C++ exceptions across the
         * native callback boundary.
         *
         * Exceptions raised during event processing are captured here
         * and rethrown through takeEvents() on the normal C++ call path.
         *
         * The stored exception is cleared when retrieved.
         */
        std::exception_ptr m_CallbackException{};

        /**
         * @brief Receives scrolling notifications from GLFW.
         *
         * Translates native scroll offsets into a backend-independent
         * ScrollEvent and appends it to the owning window's event queue.
         *
         * Exceptions are captured for later propagation through the
         * normal C++ application path.
         *
         * @param window Native GLFW window receiving the event.
         * @param x_offset Horizontal scrolling offset.
         * @param y_offset Vertical scrolling offset.
         *
         * @note Exceptions must not escape this callback.
         */
        static auto scrollCallback(GLFWwindow *window, double x_offset,
                                   double y_offset) noexcept -> void;

        /**
         * @brief Receives keyboard input notifications from GLFW.
         *
         * Translates supported native key actions into KeyEvent
         * instances and appends them to the owning window's event queue.
         *
         * Unsupported keys and actions are ignored. Exceptions are
         * captured for later propagation through takeEvents().
         *
         * @param window Native GLFW window receiving the event.
         * @param key Native GLFW key identifier.
         * @param scancode Platform-specific key scancode.
         * @param action Native key press, release, or repeat action.
         * @param modifiers Native keyboard modifier flags.
         *
         * @note Exceptions must not escape this callback.
         * @note Scancodes and modifier flags are currently unused.
         */
        static auto keyCallback(GLFWwindow *window, int key, int scancode,
                                int action, int modifiers) noexcept -> void;

        /**
         * @brief Receives mouse-button notifications from GLFW.
         *
         * Translates supported native mouse buttons and actions into
         * MouseButtonEvent instances for the owning window.
         *
         * @param window Native GLFW window receiving the event.
         * @param button Native GLFW mouse-button identifier.
         * @param action Native mouse-button press or release action.
         * @param modifiers Native modifier flags, currently unused.
         *
         * @note Exceptions are captured and must not escape this callback.
         */
        static auto mouseButtonCallback(GLFWwindow *window, int button,
                                        int action, int modifiers) noexcept
            -> void;

        /**
         * @brief Receives cursor position notifications from GLFW.
         *
         * Records absolute cursor positions in logical window units
         * as backend-independent CursorMovedEvent instances.
         *
         * @param window Native GLFW window receiving the event.
         * @param x_position Horizontal cursor position.
         * @param y_position Vertical cursor position.
         *
         * @note Exceptions are captured and must not escape this callback.
         */
        static auto cursorPositionCallback(GLFWwindow *window,
                                           double x_position,
                                           double y_position) noexcept -> void;

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

        /**
         * @brief Retrieves and removes pending events for this window.
         *
         * Returns all queued platform events in their original arrival
         * order and leaves the event queue empty and reusable.
         *
         * If a GLFW callback previously captured an exception, that
         * exception is cleared and rethrown before draining the queue.
         *
         * @return Vector containing the pending platform events,
         * or an empty vector when no events are queued.
         *
         * @throws Rethrows an exception previously captured during
         * native callback processing.
         *
         * @post On successful return, the event queue is empty.
         *
         * @note Call after platform event polling to retrieve newly
         * delivered events.
         */
        [[nodiscard]] auto takeEvents() -> std::vector<PlatformEvent>;
    };
} // namespace SNE::Engine::Platform
