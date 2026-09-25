#pragma once
#include <GLFW/glfw3.h>
#include <string>

namespace SNE::Engine::Platform {
    /**
     * @brief Specifies the dimensions of a platform window.
     */
    struct WindowSize {
        int width;
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
        [[nodiscard]] auto getNativeWindow() const -> GLFWwindow * {
            return m_Window;
        }
    };
} // namespace SNE::Engine::Platform
