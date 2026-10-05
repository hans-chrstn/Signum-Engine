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

        VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR
            fifo_latest_ready_feature{};
        fifo_latest_ready_feature.sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_MODE_FIFO_LATEST_READY_FEATURES_KHR;
        fifo_latest_ready_feature.pNext = &acceleration_feature;

        VkPhysicalDeviceFeatures2 features{};
        features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features.pNext = &fifo_latest_ready_feature;

        vkGetPhysicalDeviceFeatures2(device, &features);

        bool has_fifo_latest_ready_extension_support = false;
        bool has_acceleration_structure_support = false;
        bool has_ray_tracing_pipeline_support = false;
        bool has_ray_query_support = false;
        bool has_memory_budget_extension_support = false;

        for (const VkExtensionProperties &extension_property :
             extension_properties) {
            if (std::strcmp(
                    extension_property.extensionName,
                    VK_KHR_PRESENT_MODE_FIFO_LATEST_READY_EXTENSION_NAME) ==
                0) {
                has_fifo_latest_ready_extension_support = true;
            }

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

            if (std::strcmp(extension_property.extensionName,
                            VK_EXT_MEMORY_BUDGET_EXTENSION_NAME) == 0) {
                has_memory_budget_extension_support = true;
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
            .fifo_latest_ready_extension_supported =
                has_fifo_latest_ready_extension_support,
            .fifo_latest_ready_feature_supported = static_cast<bool>(
                fifo_latest_ready_feature.presentModeFifoLatestReady),
            .memory_budget_extension_supported =
                has_memory_budget_extension_support,
        };

        return result;
    }

    auto queryRequiredDeviceCapabilities(VkPhysicalDevice device)
        -> RequiredDeviceCapabilities {
        VkPhysicalDeviceVulkan13Features vulkan_13_features{};
        vulkan_13_features.sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;

        VkPhysicalDeviceFeatures2 feature_query_root{};
        feature_query_root.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        feature_query_root.pNext = &vulkan_13_features;
        vkGetPhysicalDeviceFeatures2(device, &feature_query_root);
        const RequiredDeviceCapabilities required_capabilities{
            .dynamic_rendering_supported =
                vulkan_13_features.dynamicRendering == VK_TRUE,
            .synchronization2_supported =
                vulkan_13_features.synchronization2 == VK_TRUE,
        };
        return required_capabilities;
    }

    auto queryPhysicalDeviceCapabilities(
        VkPhysicalDevice device,
        const std::vector<VkExtensionProperties> &extension_properties)
        -> PhysicalDeviceCapabilities {
        const VkPhysicalDeviceProperties properties =
            queryPhysicalDeviceProperties(device);
        const VkPhysicalDeviceFeatures features =
            queryPhysicalDeviceFeatures(device);
        const RequiredDeviceCapabilities required_capabilities =
            queryRequiredDeviceCapabilities(device);
        const OptionalDeviceCapabilities optional_capabilities =
            queryOptionalDeviceCapabilities(device, extension_properties);
        return {
            .properties = properties,
            .features = features,
            .required_capabilities = required_capabilities,
            .optional_capabilities = optional_capabilities,
        };
    }
} // namespace SNE::Engine::Renderer::Vulkan
