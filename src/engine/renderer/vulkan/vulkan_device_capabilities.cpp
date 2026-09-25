#include "vulkan_device_capabilities.hpp"

namespace SNE::Engine::Renderer::Vulkan {
    auto queryPhysicalDeviceProperties(VkPhysicalDevice device)
        -> VkPhysicalDeviceProperties {
        VkPhysicalDeviceProperties2 device_properties{};
        device_properties.sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        vkGetPhysicalDeviceProperties2(device, &device_properties);
        return device_properties.properties;
    }

    auto queryPhysicalDeviceFeatures(VkPhysicalDevice device)
        -> VkPhysicalDeviceFeatures {
        VkPhysicalDeviceFeatures2 features{};
        features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        vkGetPhysicalDeviceFeatures2(device, &features);
        return features.features;
    }

    auto queryPhysicalDeviceCapabilities(VkPhysicalDevice device)
        -> PhysicalDeviceCapabilities {
        VkPhysicalDeviceProperties properties =
            queryPhysicalDeviceProperties(device);
        VkPhysicalDeviceFeatures features = queryPhysicalDeviceFeatures(device);
        return {.properties = properties, .features = features};
    }
} // namespace SNE::Engine::Renderer::Vulkan
