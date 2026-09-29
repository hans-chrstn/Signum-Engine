#include "engine/renderer/vulkan/vulkan_device_extensions.hpp"
#include "engine/renderer/vulkan/vulkan_device_features.hpp"
#include <cstddef>
#include <gtest/gtest.h>
#include <vector>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

TEST(VulkanDeviceExtensionsTests,
     ReturnsRequiredExtensionsWhenOptionalFeaturesDisabled) {
    Vulkan::LogicalDeviceFeatureConfiguration configuration{};
    configuration.fifo_latest_ready_feature.presentModeFifoLatestReady =
        VK_FALSE;
    const std::vector<const char *> extensions =
        Vulkan::deriveEnabledDeviceExtensions(configuration);

    const std::vector<const char *> &required_extensions =
        Vulkan::requiredDeviceExtensions();

    ASSERT_EQ(extensions.size(), required_extensions.size());
    for (std::size_t i{}; i < required_extensions.size(); ++i) {
        EXPECT_STREQ(extensions[i], required_extensions[i]);
    }
}

TEST(VulkanDeviceExtensionsTests,
     AddsFifoLatestReadyExtensionWhenFeatureEnabled) {
    Vulkan::LogicalDeviceFeatureConfiguration configuration{};
    configuration.fifo_latest_ready_feature.presentModeFifoLatestReady =
        VK_TRUE;
    const std::vector<const char *> extensions =
        Vulkan::deriveEnabledDeviceExtensions(configuration);
    const std::vector<const char *> &required_extensions =
        Vulkan::requiredDeviceExtensions();

    ASSERT_EQ(extensions.size(), required_extensions.size() + 1);
    for (std::size_t i{}; i < required_extensions.size(); ++i) {
        EXPECT_STREQ(extensions[i], required_extensions[i]);
    }
    EXPECT_STREQ(extensions.back(),
                 VK_KHR_PRESENT_MODE_FIFO_LATEST_READY_EXTENSION_NAME);
}
