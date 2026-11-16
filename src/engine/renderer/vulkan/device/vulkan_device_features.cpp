#include "vulkan_device_features.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/renderer_capabilities.hpp"
#include "vulkan_device_capabilities.hpp"
#include <vulkan/vulkan.h>

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

    auto deriveRendererCapabilities(
        const PhysicalDeviceCapabilities &physical_capabilities,
        const LogicalDeviceFeatureConfiguration &enabled_features) noexcept
        -> RendererCapabilities {
        return RendererCapabilities{
            .acceleration_structures =
                {
                    .device_supported =
                        physical_capabilities.optional_capabilities
                            .acceleration_structures_supported,
                    .device_enabled = false,
                },
            .ray_tracing_pipelines =
                {
                    .device_supported =
                        physical_capabilities.optional_capabilities
                            .ray_tracing_pipeline_supported,
                    .device_enabled = false,
                },
            .ray_queries =
                {
                    .device_supported =
                        physical_capabilities.optional_capabilities
                            .ray_query_supported,
                    .device_enabled = false,
                },
            .gpu_memory_budget =
                {
                    .device_supported =
                        physical_capabilities.optional_capabilities
                            .memory_budget_extension_supported,
                    .device_enabled =
                        physical_capabilities.optional_capabilities
                            .memory_budget_extension_supported,
                },
            .latest_ready_presentation =
                {
                    .device_supported =
                        physical_capabilities.optional_capabilities
                            .fifo_latest_ready_extension_supported &&
                        physical_capabilities.optional_capabilities
                            .fifo_latest_ready_feature_supported,
                    .device_enabled =
                        enabled_features.fifo_latest_ready_feature
                                .presentModeFifoLatestReady == VK_TRUE &&
                        (physical_capabilities.optional_capabilities
                             .fifo_latest_ready_extension_supported &&
                         physical_capabilities.optional_capabilities
                             .fifo_latest_ready_feature_supported),
                },
        };
    }
} // namespace SNE::Engine::Renderer::Vulkan
