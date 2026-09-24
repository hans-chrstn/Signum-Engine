#pragma once
#include "engine/platform/glfw_context.hpp"
#include "engine/renderer/vulkan/vulkan_instance.hpp"

namespace SNE::Engine::Core {
    class Application {
      private:
        Platform::GlfwContext m_GlfwContext;
        Renderer::Vulkan::VulkanInstance m_VulkanInstance;

      public:
        auto run() -> void;
    };
} // namespace SNE::Engine::Core
