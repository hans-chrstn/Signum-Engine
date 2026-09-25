#pragma once
#include "engine/platform/glfw_context.hpp"
#include "engine/renderer/vulkan/vulkan_instance.hpp"

namespace SNE::Engine::Core {
    /**
     * @brief Owns and coordinates the top-level engine lifetime.
     *
     * Establishes the lifetime of engine-wide platform and rendering resources.
     * Member construction and destruction order ensure that dependencies remain
     * valid for the lifetime of the systems that require them.
     */
    class Application {
      private:
        Platform::GlfwContext m_GlfwContext;
        Renderer::Vulkan::VulkanInstance m_VulkanInstance;

      public:
        /**
         * @brief Runs the application's primary event loop.
         *
         * Creates the primary application window and processes platform events
         * until the window receives a close request.
         *
         * @throws Error::EngineError if a runtime resource required by the
         * application cannot be created.
         */
        auto run() -> void;
    };
} // namespace SNE::Engine::Core
