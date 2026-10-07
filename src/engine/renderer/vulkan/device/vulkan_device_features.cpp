#include "vulkan_device_features.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "vulkan_device_capabilities.hpp"

namespace SNE::Engine::Renderer::Vulkan {
    auto deriveLogicalDeviceFeatureConfiguration(
        const LogicalDeviceFeatureRequest &request,
        const PhysicalDeviceCapabilities &capabilities)
        -> LogicalDeviceFeatureConfiguration {
        LogicalDeviceFeatureConfiguration
            logical_device_feature_configuration{};
        if (request.fifo_latest_ready_requested &&
            capabilities.optional_capabilities
                .fifo_latest_ready_feature_supported &&
            capabilities.optional_capabilities
                .fifo_latest_ready_extension_supported) {
            logical_device_feature_configuration.fifo_latest_ready_feature
                .presentModeFifoLatestReady = VK_TRUE;
        }

        return logical_device_feature_configuration;
    }

    auto deriveLogicalDeviceFeatureRequest(
        PresentationPreference presentation_preference)
        -> LogicalDeviceFeatureRequest {
        LogicalDeviceFeatureRequest request{};
        request.fifo_latest_ready_requested =
            presentation_preference == PresentationPreference::LatestReadyVSync;

        return request;
    }

    LogicalDeviceFeatureConfiguration::LogicalDeviceFeatureConfiguration() {
        vulkan_13_features.sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        vulkan_13_features.dynamicRendering = VK_TRUE;
        vulkan_13_features.synchronization2 = VK_TRUE;
    }
} // namespace SNE::Engine::Renderer::Vulkan
