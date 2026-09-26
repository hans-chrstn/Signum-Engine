#include "engine/renderer/vulkan/vulkan_queue_families.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <vulkan/vulkan_core.h>

TEST(VulkanQueueFamilyTests, FindGraphicsQueueFamilyReturnsFirstMatchingIndex) {
    VkQueueFamilyProperties test_property{};
    test_property.queueFlags = VK_QUEUE_GRAPHICS_BIT;

    const std::vector<VkQueueFamilyProperties> properties{
        test_property,
    };

    const auto result =
        SNE::Engine::Renderer::Vulkan::findGraphicsQueueFamily(properties);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result, std::optional<std::uint32_t>{0});
}

TEST(VulkanQueueFamilyTests, FindGraphicsQueueFamilyFindsLaterMatchingIndex) {
    VkQueueFamilyProperties graphics_family{};
    graphics_family.queueFlags = VK_QUEUE_GRAPHICS_BIT;

    VkQueueFamilyProperties compute_family{};
    compute_family.queueFlags = VK_QUEUE_COMPUTE_BIT;

    VkQueueFamilyProperties transfer_family{};
    transfer_family.queueFlags = VK_QUEUE_TRANSFER_BIT;

    const std::vector<VkQueueFamilyProperties> properties{
        compute_family,
        transfer_family,
        graphics_family,
    };

    const auto result =
        SNE::Engine::Renderer::Vulkan::findGraphicsQueueFamily(properties);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result, std::optional<std::uint32_t>{2});
}

TEST(VulkanQueueFamilyTests, FindGraphicsQueueFamilyPrefersFirstMatchingIndex) {
    VkQueueFamilyProperties first_graphics_family{};
    first_graphics_family.queueFlags =
        VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;

    VkQueueFamilyProperties second_graphics_family{};
    second_graphics_family.queueFlags =
        VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT;

    VkQueueFamilyProperties transfer_family{};
    transfer_family.queueFlags = VK_QUEUE_TRANSFER_BIT;

    const std::vector<VkQueueFamilyProperties> properties{
        first_graphics_family,
        second_graphics_family,
        transfer_family,
    };

    const auto result =
        SNE::Engine::Renderer::Vulkan::findGraphicsQueueFamily(properties);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result, std::optional<std::uint32_t>{0});
}

TEST(VulkanQueueFamilyTests,
     FindGraphicsQueueFamilyReturnsNulloptWhenUnsupported) {
    VkQueueFamilyProperties compute_transfer_family{};
    compute_transfer_family.queueFlags =
        VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;

    VkQueueFamilyProperties compute_family{};
    compute_family.queueFlags = VK_QUEUE_COMPUTE_BIT;

    VkQueueFamilyProperties transfer_family{};
    transfer_family.queueFlags = VK_QUEUE_TRANSFER_BIT;

    const std::vector<VkQueueFamilyProperties> properties{
        compute_family,
        transfer_family,
        compute_transfer_family,
    };

    const auto result =
        SNE::Engine::Renderer::Vulkan::findGraphicsQueueFamily(properties);

    EXPECT_EQ(result, std::nullopt);
}

TEST(VulkanQueueFamilyTests,
     FindGraphicsQueueFamilyReturnsNulloptForEmptyInput) {
    const std::vector<VkQueueFamilyProperties> properties{};

    const auto result =
        SNE::Engine::Renderer::Vulkan::findGraphicsQueueFamily(properties);

    EXPECT_EQ(result, std::nullopt);
}
