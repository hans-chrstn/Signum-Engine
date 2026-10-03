#pragma once

#include "application_configuration.hpp"
#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/vulkan/vulkan_renderer.hpp"

namespace SNE::Engine::Core {
    /**
     * @brief Owns and coordinates the top-level engine lifetime.
     *
     * Establishes the lifetime of engine-wide configuration, platform, window,
     * and rendering systems. Members are declared in dependency order so
     * construction occurs from lower-level dependencies to higher-level systems
     * and destruction occurs safely in reverse order.
     *
     * Rendering backend initialization and Vulkan resource ownership are
     * delegated to the renderer rather than managed directly by Application.
     */
    class Application {
      private:
        ApplicationConfiguration m_Configuration;
        Platform::GlfwContext m_GlfwContext;
        Platform::Window m_Window;
        Renderer::Vulkan::VulkanRenderer m_Renderer;

      public:
        /**
         * @brief Constructs the application and its engine-wide systems.
         *
         * Stores the supplied application configuration, initializes the
         * platform context, creates the primary application window, and
         * initializes the renderer for that window using the configured
         * presentation and diagnostic policies.
         *
         * @param configuration Top-level configuration controlling application
         * startup policy. The default value uses Signum's standard application
         * settings.
         *
         * @throws Error::EngineError if a required platform, window, or
         * rendering resource cannot be initialized.
         */
        explicit Application(ApplicationConfiguration configuration = {});

        /**
         * @brief Runs the application's primary event and rendering loop.
         *
         * Processes platform events and requests one renderer frame repeatedly
         * until the primary application window receives a close request.
         */
        auto run() -> void;
    };
} // namespace SNE::Engine::Core
