#pragma once
#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/vulkan/vulkan_instance.hpp"
#include "engine/renderer/vulkan/vulkan_surface.hpp"

namespace SNE::Engine::Core {
    /**
     * @brief Owns and coordinates the top-level engine lifetime.
     *
     * Establishes the lifetime of engine-wide platform, window, and rendering
     * resources. Members are declared in dependency order so construction
     * occurs from lower-level dependencies to higher-level systems and
     * destruction occurs safely in reverse order.
     */
    class Application {
      private:
        Platform::GlfwContext m_GlfwContext;
        Platform::Window m_Window;
        Renderer::Vulkan::VulkanInstance m_VulkanInstance;
        Renderer::Vulkan::VulkanSurface m_VulkanSurface;

      public:
        /**
         * @brief Constructs the application and its engine-wide resources.
         *
         * Initializes the platform context, creates the primary application
         * window, establishes the Vulkan instance, and creates the Vulkan
         * presentation surface associated with the window.
         *
         * @throws Error::EngineError if a required platform, window, or Vulkan
         * resource cannot be initialized.
         */
        Application();

        /**
         * @brief Runs the application's primary event loop.
         *
         * Processes platform events until the primary application window
         * receives a close request.
         */
        auto run() -> void;
    };
} // namespace SNE::Engine::Core
