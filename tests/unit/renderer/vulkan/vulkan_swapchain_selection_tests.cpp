#include "engine/renderer/vulkan/vulkan_swapchain_selection.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

TEST(VulkanSwapchainSelectionTests,
     SelectSurfaceFormatReturnsPreferredFormatWhenFirst) {
    const VkSurfaceFormatKHR first_test_format{
        .format = VK_FORMAT_B8G8R8A8_SRGB,
        .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
    };

    const VkSurfaceFormatKHR second_test_format{
        .format = VK_FORMAT_R8G8B8A8_SRGB,
        .colorSpace = VK_COLOR_SPACE_BT709_NONLINEAR_EXT,
    };

    const std::vector<VkSurfaceFormatKHR> test_formats{
        first_test_format,
        second_test_format,
    };

    const VkSurfaceFormatKHR selected =
        Vulkan::selectSurfaceFormat(test_formats);

    EXPECT_EQ(selected.format, first_test_format.format);
    EXPECT_EQ(selected.colorSpace, first_test_format.colorSpace);
}

TEST(VulkanSwapchainSelectionTests,
     SelectSurfaceFormatFindsPreferredFormatLater) {
    const VkSurfaceFormatKHR first_test_format{
        .format = VK_FORMAT_R8G8B8A8_SRGB,
        .colorSpace = VK_COLOR_SPACE_BT709_NONLINEAR_EXT,
    };

    const VkSurfaceFormatKHR second_test_format{
        .format = VK_FORMAT_B8G8R8A8_SRGB,
        .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
    };

    const std::vector<VkSurfaceFormatKHR> test_formats{
        first_test_format,
        second_test_format,
    };

    const VkSurfaceFormatKHR selected =
        Vulkan::selectSurfaceFormat(test_formats);

    EXPECT_EQ(selected.format, second_test_format.format);
    EXPECT_EQ(selected.colorSpace, second_test_format.colorSpace);
}

TEST(VulkanSwapchainSelectionTests,
     SelectSurfaceFormatReturnsFirstFormatWhenPreferredUnavailable) {
    const VkSurfaceFormatKHR first_test_format{
        .format = VK_FORMAT_R8_SRGB,
        .colorSpace = VK_COLOR_SPACE_BT709_LINEAR_EXT,
    };

    const VkSurfaceFormatKHR second_test_format{
        .format = VK_FORMAT_B8G8R8A8_SRGB,
        .colorSpace = VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT,
    };

    const std::vector<VkSurfaceFormatKHR> test_formats{
        first_test_format,
        second_test_format,
    };

    const VkSurfaceFormatKHR selected =
        Vulkan::selectSurfaceFormat(test_formats);

    EXPECT_EQ(selected.format, first_test_format.format);
    EXPECT_EQ(selected.colorSpace, first_test_format.colorSpace);
}

TEST(VulkanSwapchainSelectionTests,
     SelectSurfaceFormatRequiresBothFormatAndColorSpaceToMatch) {
    const VkSurfaceFormatKHR first_test_format{
        .format = VK_FORMAT_B8G8R8A8_SRGB,
        .colorSpace = VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT,
    };

    const VkSurfaceFormatKHR second_test_format{
        .format = VK_FORMAT_B8G8R8A8_SRGB,
        .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
    };

    const std::vector<VkSurfaceFormatKHR> test_formats{
        first_test_format,
        second_test_format,
    };

    const VkSurfaceFormatKHR selected =
        Vulkan::selectSurfaceFormat(test_formats);

    EXPECT_EQ(selected.format, second_test_format.format);
    EXPECT_EQ(selected.colorSpace, second_test_format.colorSpace);
}
