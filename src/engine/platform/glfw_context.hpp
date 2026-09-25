#pragma once

/**
 *
 * Manages the GLFW library lifetime
 *
 */
namespace SNE::Engine::Platform {
    /**
     * @brief Owns the process-wide GLFW library lifetime.
     *
     * Construction initializes GLFW and destruction terminates it.
     *
     * A GlfwContext must outlive all engine resources that depend on GLFW,
     * including Window objects and operations that query GLFW for Vulkan
     * integration information.
     *
     * The type is non-copyable because it represents ownership of process-wide
     * library state.
     */
    class GlfwContext {
      public:
        /**
         * @brief Initializes GLFW.
         *
         * @throws Core::Error::EngineError if GLFW initialization fails.
         */
        GlfwContext();

        /**
         * @brief Terminates GLFW.
         */
        ~GlfwContext();

        GlfwContext(const GlfwContext &) = delete;
        auto operator=(const GlfwContext &) -> GlfwContext & = delete;
    };
} // namespace SNE::Engine::Platform
