#include "engine/renderer/vulkan/vulkan_device_capabilities.hpp"
#include "engine/renderer/vulkan/vulkan_device_extensions.hpp"
#include "engine/renderer/vulkan/vulkan_device_features.hpp"
#include <cstddef>
#include <gtest/gtest.h>
#include <span>
#include <vector>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

TEST(VulkanDeviceExtensionsTests,
     ReturnsRequiredExtensionsWhenOptionalFeaturesDisabled) {
    Vulkan::LogicalDeviceFeatureConfiguration configuration{};
    configuration.fifo_latest_ready_feature.presentModeFifoLatestReady =
        VK_FALSE;
    const Vulkan::OptionalDeviceCapabilities optional_capabilities{};
    const std::vector<const char *> extensions =
        Vulkan::deriveEnabledDeviceExtensions(configuration,
                                              optional_capabilities);

    std::span<const char *const> required_extensions =
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
    const Vulkan::OptionalDeviceCapabilities optional_capabilities{};
    const std::vector<const char *> extensions =
        Vulkan::deriveEnabledDeviceExtensions(configuration,
                                              optional_capabilities);
    std::span<const char *const> required_extensions =
        Vulkan::requiredDeviceExtensions();

    ASSERT_EQ(extensions.size(), required_extensions.size() + 1);
    for (std::size_t i{}; i < required_extensions.size(); ++i) {
        EXPECT_STREQ(extensions[i], required_extensions[i]);
    }
    EXPECT_STREQ(extensions.back(),
                 VK_KHR_PRESENT_MODE_FIFO_LATEST_READY_EXTENSION_NAME);
}

TEST(VulkanDeviceExtensionsTests, AddsMemoryBudgetExtensionWhenSupported) {
    const Vulkan::LogicalDeviceFeatureConfiguration configuration{};
    Vulkan::OptionalDeviceCapabilities optional_capabilities{};
    optional_capabilities.memory_budget_extension_supported = true;
    const std::vector<const char *> extensions =
        Vulkan::deriveEnabledDeviceExtensions(configuration,
                                              optional_capabilities);
    std::span<const char *const> required_extensions =
        Vulkan::requiredDeviceExtensions();

    ASSERT_EQ(extensions.size(), required_extensions.size() + 1);
    for (std::size_t i{}; i < required_extensions.size(); ++i) {
        EXPECT_STREQ(extensions[i], required_extensions[i]);
    }

    EXPECT_STREQ(extensions.back(), VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
}

TEST(VulkanDeviceExtensionsTests,
     DoesNotAddMemoryBudgetExtensionWhenUnsupported) {
    const Vulkan::LogicalDeviceFeatureConfiguration configuration{};
    const Vulkan::OptionalDeviceCapabilities optional_capabilities{};
    const std::vector<const char *> extensions =
        Vulkan::deriveEnabledDeviceExtensions(configuration,
                                              optional_capabilities);
    std::span<const char *const> required_extensions =
        Vulkan::requiredDeviceExtensions();

    ASSERT_EQ(extensions.size(), required_extensions.size());
    for (std::size_t i{}; i < required_extensions.size(); ++i) {
        EXPECT_STRNE(extensions[i], VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
    }
}
