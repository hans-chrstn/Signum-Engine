#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/renderer_capabilities.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_capabilities.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_features.hpp"
#include <gtest/gtest.h>
#include <vulkan/vulkan.h>

namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = Renderer::Vulkan;

TEST(VulkanDeviceFeaturesTests, DoesNotEnableFifoLatestReadyWhenNotRequested) {
    const Vulkan::LogicalDeviceFeatureRequest request{
        .fifo_latest_ready_requested = false,
    };

    Vulkan::PhysicalDeviceCapabilities capabilities{};
    capabilities.optional_capabilities.fifo_latest_ready_extension_supported =
        true;
    capabilities.optional_capabilities.fifo_latest_ready_feature_supported =
        true;

    const Vulkan::LogicalDeviceFeatureConfiguration configuration =
        Vulkan::deriveLogicalDeviceFeatureConfiguration(request, capabilities);

    EXPECT_EQ(
        configuration.fifo_latest_ready_feature.presentModeFifoLatestReady,
        VK_FALSE);
}

TEST(VulkanDeviceFeaturesTests,
     DoesNotEnableFifoLatestReadyWhenExtensionUnsupported) {
    const Vulkan::LogicalDeviceFeatureRequest request{
        .fifo_latest_ready_requested = true,
    };

    Vulkan::PhysicalDeviceCapabilities capabilities{};
    capabilities.optional_capabilities.fifo_latest_ready_extension_supported =
        false;
    capabilities.optional_capabilities.fifo_latest_ready_feature_supported =
        true;

    const Vulkan::LogicalDeviceFeatureConfiguration configuration =
        Vulkan::deriveLogicalDeviceFeatureConfiguration(request, capabilities);

    EXPECT_EQ(
        configuration.fifo_latest_ready_feature.presentModeFifoLatestReady,
        VK_FALSE);
}

TEST(VulkanDeviceFeaturesTests,
     DoesNotEnableFifoLatestReadyWhenFeatureUnsupported) {
    const Vulkan::LogicalDeviceFeatureRequest request{
        .fifo_latest_ready_requested = true,
    };

    Vulkan::PhysicalDeviceCapabilities capabilities{};
    capabilities.optional_capabilities.fifo_latest_ready_extension_supported =
        true;
    capabilities.optional_capabilities.fifo_latest_ready_feature_supported =
        false;

    const Vulkan::LogicalDeviceFeatureConfiguration configuration =
        Vulkan::deriveLogicalDeviceFeatureConfiguration(request, capabilities);

    EXPECT_EQ(
        configuration.fifo_latest_ready_feature.presentModeFifoLatestReady,
        VK_FALSE);
}

TEST(VulkanDeviceFeaturesTests,
     EnablesFifoLatestReadyWhenRequestedAndFullySupported) {
    const Vulkan::LogicalDeviceFeatureRequest request{
        .fifo_latest_ready_requested = true,
    };

    Vulkan::PhysicalDeviceCapabilities capabilities{};
    capabilities.optional_capabilities.fifo_latest_ready_extension_supported =
        true;
    capabilities.optional_capabilities.fifo_latest_ready_feature_supported =
        true;

    const Vulkan::LogicalDeviceFeatureConfiguration configuration =
        Vulkan::deriveLogicalDeviceFeatureConfiguration(request, capabilities);

    EXPECT_EQ(
        configuration.fifo_latest_ready_feature.presentModeFifoLatestReady,
        VK_TRUE);
}

TEST(VulkanDeviceFeaturesTests,
     ReturnsFifoLatestReadyRequestForLatestReadyVSync) {
    const Renderer::PresentationPreference presentation_preference =
        Renderer::PresentationPreference::LatestReadyVSync;

    const Vulkan::LogicalDeviceFeatureRequest request =
        Vulkan::deriveLogicalDeviceFeatureRequest(presentation_preference);
    EXPECT_TRUE(request.fifo_latest_ready_requested);
}

TEST(VulkanDeviceFeaturesTests, DoesNotRequestFifoLatestReadyForVSync) {
    const Renderer::PresentationPreference presentation_preference =
        Renderer::PresentationPreference::VSync;

    const Vulkan::LogicalDeviceFeatureRequest request =
        Vulkan::deriveLogicalDeviceFeatureRequest(presentation_preference);
    EXPECT_FALSE(request.fifo_latest_ready_requested);
}

TEST(VulkanDeviceFeaturesTests,
     ReportsAllOptionalCapabilitiesDisabledWhenUnsupported) {
    const Vulkan::PhysicalDeviceCapabilities physical_capabilities{};
    const Vulkan::LogicalDeviceFeatureConfiguration enabled_features{};

    const Renderer::RendererCapabilities capabilities =
        Vulkan::deriveRendererCapabilities(physical_capabilities,
                                           enabled_features);

    EXPECT_FALSE(capabilities.acceleration_structures.device_supported);
    EXPECT_FALSE(capabilities.acceleration_structures.device_enabled);

    EXPECT_FALSE(capabilities.ray_tracing_pipelines.device_supported);
    EXPECT_FALSE(capabilities.ray_tracing_pipelines.device_enabled);

    EXPECT_FALSE(capabilities.ray_queries.device_supported);
    EXPECT_FALSE(capabilities.ray_queries.device_enabled);

    EXPECT_FALSE(capabilities.gpu_memory_budget.device_supported);
    EXPECT_FALSE(capabilities.gpu_memory_budget.device_enabled);

    EXPECT_FALSE(capabilities.latest_ready_presentation.device_supported);
    EXPECT_FALSE(capabilities.latest_ready_presentation.device_enabled);
}

TEST(VulkanDeviceFeaturesTests,
     ReportsSupportedRayTracingCapabilitiesWithoutEnablingThem) {
    Vulkan::PhysicalDeviceCapabilities physical_capabilities{};
    physical_capabilities.optional_capabilities
        .acceleration_structures_supported = true;
    physical_capabilities.optional_capabilities.ray_tracing_pipeline_supported =
        true;
    physical_capabilities.optional_capabilities.ray_query_supported = true;

    const Vulkan::LogicalDeviceFeatureConfiguration enabled_features{};

    const Renderer::RendererCapabilities capabilities =
        Vulkan::deriveRendererCapabilities(physical_capabilities,
                                           enabled_features);

    EXPECT_TRUE(capabilities.acceleration_structures.device_supported);
    EXPECT_FALSE(capabilities.acceleration_structures.device_enabled);

    EXPECT_TRUE(capabilities.ray_tracing_pipelines.device_supported);
    EXPECT_FALSE(capabilities.ray_tracing_pipelines.device_enabled);

    EXPECT_TRUE(capabilities.ray_queries.device_supported);
    EXPECT_FALSE(capabilities.ray_queries.device_enabled);
}

TEST(VulkanDeviceFeaturesTests,
     ReportsMemoryBudgetEnabledWhenExtensionSupported) {
    Vulkan::PhysicalDeviceCapabilities physical_capabilities{};
    physical_capabilities.optional_capabilities
        .memory_budget_extension_supported = true;

    const Vulkan::LogicalDeviceFeatureConfiguration enabled_features{};

    const Renderer::RendererCapabilities capabilities =
        Vulkan::deriveRendererCapabilities(physical_capabilities,
                                           enabled_features);

    EXPECT_TRUE(capabilities.gpu_memory_budget.device_supported);
    EXPECT_TRUE(capabilities.gpu_memory_budget.device_enabled);
}

TEST(VulkanDeviceFeaturesTests,
     ReportsLatestReadySupportedButDisabledWhenNotRequested) {
    Vulkan::PhysicalDeviceCapabilities physical_capabilities{};
    physical_capabilities.optional_capabilities
        .fifo_latest_ready_extension_supported = true;
    physical_capabilities.optional_capabilities
        .fifo_latest_ready_feature_supported = true;

    const Vulkan::LogicalDeviceFeatureRequest request{
        .fifo_latest_ready_requested = false,
    };

    const Vulkan::LogicalDeviceFeatureConfiguration enabled_features =
        Vulkan::deriveLogicalDeviceFeatureConfiguration(request,
                                                        physical_capabilities);

    const Renderer::RendererCapabilities capabilities =
        Vulkan::deriveRendererCapabilities(physical_capabilities,
                                           enabled_features);

    EXPECT_TRUE(capabilities.latest_ready_presentation.device_supported);
    EXPECT_FALSE(capabilities.latest_ready_presentation.device_enabled);
}

TEST(VulkanDeviceFeaturesTests,
     ReportsLatestReadyEnabledWhenRequestedAndSupported) {
    Vulkan::PhysicalDeviceCapabilities physical_capabilities{};
    physical_capabilities.optional_capabilities
        .fifo_latest_ready_extension_supported = true;
    physical_capabilities.optional_capabilities
        .fifo_latest_ready_feature_supported = true;

    const Vulkan::LogicalDeviceFeatureRequest request{
        .fifo_latest_ready_requested = true,
    };

    const Vulkan::LogicalDeviceFeatureConfiguration enabled_features =
        Vulkan::deriveLogicalDeviceFeatureConfiguration(request,
                                                        physical_capabilities);

    const Renderer::RendererCapabilities capabilities =
        Vulkan::deriveRendererCapabilities(physical_capabilities,
                                           enabled_features);

    EXPECT_TRUE(capabilities.latest_ready_presentation.device_supported);
    EXPECT_TRUE(capabilities.latest_ready_presentation.device_enabled);
}

TEST(VulkanDeviceFeaturesTests,
     DoesNotReportLatestReadyEnabledWithoutExtensionSupport) {
    Vulkan::PhysicalDeviceCapabilities physical_capabilities{};
    physical_capabilities.optional_capabilities
        .fifo_latest_ready_extension_supported = false;
    physical_capabilities.optional_capabilities
        .fifo_latest_ready_feature_supported = true;

    Vulkan::LogicalDeviceFeatureConfiguration enabled_features{};

    enabled_features.fifo_latest_ready_feature.presentModeFifoLatestReady =
        VK_TRUE;

    const Renderer::RendererCapabilities capabilities =
        Vulkan::deriveRendererCapabilities(physical_capabilities,
                                           enabled_features);

    EXPECT_FALSE(capabilities.latest_ready_presentation.device_supported);
    EXPECT_FALSE(capabilities.latest_ready_presentation.device_enabled);
}

TEST(VulkanDeviceFeaturesTests,
     DoesNotReportLatestReadyEnabledWithoutFeatureSupport) {
    Vulkan::PhysicalDeviceCapabilities physical_capabilities{};
    physical_capabilities.optional_capabilities
        .fifo_latest_ready_extension_supported = true;
    physical_capabilities.optional_capabilities
        .fifo_latest_ready_feature_supported = false;

    Vulkan::LogicalDeviceFeatureConfiguration enabled_features{};

    enabled_features.fifo_latest_ready_feature.presentModeFifoLatestReady =
        VK_TRUE;

    const Renderer::RendererCapabilities capabilities =
        Vulkan::deriveRendererCapabilities(physical_capabilities,
                                           enabled_features);

    EXPECT_FALSE(capabilities.latest_ready_presentation.device_supported);
    EXPECT_FALSE(capabilities.latest_ready_presentation.device_enabled);
}
