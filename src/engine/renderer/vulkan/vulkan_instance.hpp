#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    class VulkanInstance {
      private:
        VkInstance m_Instance{VK_NULL_HANDLE};
        VkDebugUtilsMessengerEXT m_DebugMessenger{VK_NULL_HANDLE};
        auto createInstance() -> void;
        [[nodiscard]] static auto checkValidationLayerSupport() -> bool;
        [[nodiscard]] static auto getRequiredExtensions()
            -> std::vector<const char *>;
        auto setupDebugMessenger() -> void;
        static VKAPI_ATTR VkBool32 VKAPI_CALL
        // NOLINTNEXTLINE(modernize-use-trailing-return-type)
        debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                      VkDebugUtilsMessageTypeFlagsEXT message_type,
                      const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                      void *user_data);
        auto destroyDebugMessenger() noexcept -> void;
        [[nodiscard]] static auto makeDebugMessengerCreateInfo()
            -> VkDebugUtilsMessengerCreateInfoEXT;

      public:
        VulkanInstance();
        ~VulkanInstance();

        VulkanInstance(const VulkanInstance &) = delete;
        auto operator=(const VulkanInstance &) -> VulkanInstance & = delete;
    };
} // namespace SNE::Engine::Renderer::Vulkan
