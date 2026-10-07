#include "engine/renderer/presentation_preference.hpp"
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
