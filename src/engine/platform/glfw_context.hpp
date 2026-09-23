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
        auto operator=(const GlfwContext &) -> GlfwContext & = delete;
    };
} // namespace SNE::Engine::Platform
