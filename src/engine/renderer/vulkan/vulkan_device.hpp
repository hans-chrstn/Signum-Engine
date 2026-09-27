#include "vulkan_device_features.hpp"
#include "vulkan_queue_requests.hpp"
#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    class VulkanDevice {
      private:
        VkDevice m_Vulkan_Device{VK_NULL_HANDLE};

      public:
        VulkanDevice(
            VkPhysicalDevice physical_device,
            const std::vector<QueueFamilyRequest> &queue_family_requests,
            const LogicalDeviceFeatureConfiguration
                &logical_device_configuration,
            const std::vector<VkExtensionProperties> &extensions);
        ~VulkanDevice();
        VulkanDevice(const VulkanDevice &) = delete;
        auto operator=(const VulkanDevice &) -> VulkanDevice & = delete;
    };
} // namespace SNE::Engine::Renderer::Vulkan
