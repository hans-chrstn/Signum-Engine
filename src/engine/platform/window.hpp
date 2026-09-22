#pragma once
#include <GLFW/glfw3.h>
#include <string>

namespace SNE::Engine::Platform {
    class Window {
      private:
        int m_Width;
        int m_Height;
        std::string m_Title;
        GLFWwindow *m_Window = nullptr;

      public:
        Window(int width, int height, const std::string &title);
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        bool shouldClose() const;
        void waitEvents() const;

        GLFWwindow *getNativeWindow() const {
            return m_Window;
        }
    };
} // namespace SNE::Engine::Platform
