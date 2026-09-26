#include "vulkan_device_capabilities.hpp"
#include <cstring>

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

    auto queryOptionalDeviceCapabilities(
        VkPhysicalDevice device,
        const std::vector<VkExtensionProperties> &extension_properties)
        -> OptionalDeviceCapabilities {
        VkPhysicalDeviceRayQueryFeaturesKHR ray_query_feature{};
        ray_query_feature.sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;

        VkPhysicalDeviceRayTracingPipelineFeaturesKHR
            ray_tracing_pipeline_feature{};
        ray_tracing_pipeline_feature.sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
        ray_tracing_pipeline_feature.pNext = &ray_query_feature;

        VkPhysicalDeviceAccelerationStructureFeaturesKHR acceleration_feature{};
        acceleration_feature.sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
        acceleration_feature.pNext = &ray_tracing_pipeline_feature;

        VkPhysicalDeviceFeatures2 features{};
        features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features.pNext = &acceleration_feature;

        vkGetPhysicalDeviceFeatures2(device, &features);

        bool has_acceleration_structure_support = false;
        bool has_ray_tracing_pipeline_support = false;
        bool has_ray_query_support = false;

        for (const VkExtensionProperties &extension_property :
             extension_properties) {
            if (std::strcmp(extension_property.extensionName,
                            VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) ==
                0) {
                has_acceleration_structure_support = true;
            }

            if (std::strcmp(extension_property.extensionName,
                            VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) == 0) {
                has_ray_tracing_pipeline_support = true;
            }

            if (std::strcmp(extension_property.extensionName,
                            VK_KHR_RAY_QUERY_EXTENSION_NAME) == 0) {
                has_ray_query_support = true;
            }
        }

        const bool acceleration_structures_supported =
            has_acceleration_structure_support &&
            static_cast<bool>(acceleration_feature.accelerationStructure);

        const OptionalDeviceCapabilities result{
            .acceleration_structures_supported =
                acceleration_structures_supported,
            .ray_tracing_pipeline_supported =
                acceleration_structures_supported &&
                has_ray_tracing_pipeline_support &&
                static_cast<bool>(
                    ray_tracing_pipeline_feature.rayTracingPipeline),
            .ray_query_supported =
                acceleration_structures_supported && has_ray_query_support &&
                static_cast<bool>(ray_query_feature.rayQuery),
        };

        return result;
    }

    auto queryPhysicalDeviceCapabilities(
        VkPhysicalDevice device,
        const std::vector<VkExtensionProperties> &extension_properties)
        -> PhysicalDeviceCapabilities {
        VkPhysicalDeviceProperties properties =
            queryPhysicalDeviceProperties(device);
        VkPhysicalDeviceFeatures features = queryPhysicalDeviceFeatures(device);
        OptionalDeviceCapabilities optional_capabilities =
            queryOptionalDeviceCapabilities(device, extension_properties);
        return {.properties = properties,
                .features = features,
                .optional_capabilities = optional_capabilities};
    }
} // namespace SNE::Engine::Renderer::Vulkan
