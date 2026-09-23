#pragma once
#include <GLFW/glfw3.h>
#include <string>

namespace SNE::Engine::Platform {
    struct WindowSize {
        int width;
        int height;
    };

    class Window {
      private:
        WindowSize m_Size;
        std::string m_Title;
        GLFWwindow *m_Window = nullptr;

      public:
        Window(WindowSize size, std::string title);
        ~Window();

        Window(const Window &) = delete;
        auto operator=(const Window &) -> Window & = delete;

        [[nodiscard]] auto shouldClose() const -> bool;

        static void waitEvents();

        [[nodiscard]] auto getNativeWindow() const -> GLFWwindow * {
            return m_Window;
        }
    };
} // namespace SNE::Engine::Platform
