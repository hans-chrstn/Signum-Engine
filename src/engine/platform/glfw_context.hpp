#pragma once

/**
 *
 * Manages the GLFW library lifetime
 *
 */
namespace SNE::Engine::Platform {
    class GlfwContext {
      public:
        GlfwContext();
        ~GlfwContext();

        GlfwContext(const GlfwContext &) = delete;
        GlfwContext &operator=(const GlfwContext &) = delete;
    };
} // namespace SNE::Engine::Platform
