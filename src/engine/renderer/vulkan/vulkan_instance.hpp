#pragma once

#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    class VulkanInstance {
      private:
        VkInstance m_Instance{VK_NULL_HANDLE};

      public:
        VulkanInstance();
        ~VulkanInstance();

        VulkanInstance(const VulkanInstance &) = delete;
        auto operator=(const VulkanInstance &) -> VulkanInstance & = delete;
    };
} // namespace SNE::Engine::Renderer::Vulkan
