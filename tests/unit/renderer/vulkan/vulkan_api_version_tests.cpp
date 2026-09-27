#include "engine/renderer/vulkan/vulkan_api_version.hpp"
#include <gtest/gtest.h>
#include <vulkan/vulkan.h>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

TEST(VulkanApiVersionTests, ReturnsTrueWhenApiVersionEqualsRequiredVersion) {
    EXPECT_TRUE(
        Vulkan::supportsRequiredApiVersion(VK_MAKE_API_VERSION(0, 1, 4, 0)));
}

TEST(VulkanApiVersionTests, ReturnsTrueWhenApiVersionExceedsRequiredVersion) {
    EXPECT_TRUE(
        Vulkan::supportsRequiredApiVersion(VK_MAKE_API_VERSION(0, 1, 4, 1)));
}

TEST(VulkanApiVersionTests, ReturnsFalseWhenApiVersionIsBelowRequiredVersion) {
    EXPECT_FALSE(
        Vulkan::supportsRequiredApiVersion(VK_MAKE_API_VERSION(0, 1, 3, 999)));
}
