#pragma once

namespace SNE::Engine::Core::Logging {
    class Logger;
}

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
         * @brief Initializes GLFW with optional engine diagnostic logging.
         *
         * Establishes the process-wide GLFW library lifetime and optionally
         * receives access to an externally owned diagnostic logger.
         *
         * The supplied logger is borrowed and must remain valid throughout
         * any logging operations performed by this context.
         *
         * @param logger Optional non-owning logger used for platform
         * diagnostics. A null pointer disables logger-based diagnostics.
         *
         * @throws Core::Error::EngineError if GLFW initialization fails.
         *
         * @note The context does not own or control the logger's lifetime.
         */
        explicit GlfwContext(Core::Logging::Logger *logger = nullptr);

        /**
         * @brief Terminates GLFW.
         */
        ~GlfwContext();

        GlfwContext(const GlfwContext &) = delete;
        auto operator=(const GlfwContext &) -> GlfwContext & = delete;
    };
} // namespace SNE::Engine::Platform
