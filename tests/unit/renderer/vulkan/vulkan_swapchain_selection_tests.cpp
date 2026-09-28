#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/vulkan/vulkan_swapchain_selection.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <vulkan/vulkan.h>

namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = Renderer::Vulkan;

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

TEST(VulkanSwapchainSelectionTests, SelectPresentationModeReturnsFifoForVSync) {
    const std::vector<VkPresentModeKHR> presentation_modes{
        VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_MAILBOX_KHR,
        VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR,
        VK_PRESENT_MODE_FIFO_LATEST_READY_KHR};

    EXPECT_EQ(
        Vulkan::selectPresentationMode(Renderer::PresentationPreference::VSync,
                                       presentation_modes, false),
        VK_PRESENT_MODE_FIFO_KHR);
}

TEST(VulkanSwapchainSelectionTests,
     SelectPresentationModeReturnsMailboxForLowLatencyVSyncWhenAvailable) {
    const std::vector<VkPresentModeKHR> presentation_modes{
        VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_MAILBOX_KHR,
        VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR,
        VK_PRESENT_MODE_FIFO_LATEST_READY_KHR};

    EXPECT_EQ(Vulkan::selectPresentationMode(
                  Renderer::PresentationPreference::LowLatencyVSync,
                  presentation_modes, false),
              VK_PRESENT_MODE_MAILBOX_KHR);
}

TEST(
    VulkanSwapchainSelectionTests,
    SelectPresentationModeReturnsFifoForLowLatencyVSyncWhenMailboxUnavailable) {
    const std::vector<VkPresentModeKHR> presentation_modes{
        VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR,
        VK_PRESENT_MODE_FIFO_RELAXED_KHR,
        VK_PRESENT_MODE_FIFO_LATEST_READY_KHR};

    EXPECT_EQ(Vulkan::selectPresentationMode(
                  Renderer::PresentationPreference::LowLatencyVSync,
                  presentation_modes, false),
              VK_PRESENT_MODE_FIFO_KHR);
}

TEST(VulkanSwapchainSelectionTests,
     SelectPresentationModeReturnsFifoRelaxedForAdaptiveVSyncWhenAvailable) {
    const std::vector<VkPresentModeKHR> presentation_modes{
        VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_MAILBOX_KHR,
        VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR,
        VK_PRESENT_MODE_FIFO_LATEST_READY_KHR};

    EXPECT_EQ(Vulkan::selectPresentationMode(
                  Renderer::PresentationPreference::AdaptiveVSync,
                  presentation_modes, false),
              VK_PRESENT_MODE_FIFO_RELAXED_KHR);
}

TEST(
    VulkanSwapchainSelectionTests,
    SelectPresentationModeReturnsFifoForAdaptiveVSyncWhenFifoRelaxedUnavailable) {
    const std::vector<VkPresentModeKHR> presentation_modes{
        VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_MAILBOX_KHR,
        VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_LATEST_READY_KHR};

    EXPECT_EQ(Vulkan::selectPresentationMode(
                  Renderer::PresentationPreference::AdaptiveVSync,
                  presentation_modes, false),
              VK_PRESENT_MODE_FIFO_KHR);
}

TEST(VulkanSwapchainSelectionTests,
     SelectPresentationModeReturnsImmediateForAllowTearingWhenAvailable) {
    const std::vector<VkPresentModeKHR> presentation_modes{
        VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_MAILBOX_KHR,
        VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR,
        VK_PRESENT_MODE_FIFO_LATEST_READY_KHR};

    EXPECT_EQ(Vulkan::selectPresentationMode(
                  Renderer::PresentationPreference::AllowTearing,
                  presentation_modes, false),
              VK_PRESENT_MODE_IMMEDIATE_KHR);
}

TEST(
    VulkanSwapchainSelectionTests,
    SelectPresentationModeReturnsMailboxForAllowTearingWhenImmediateUnavailable) {
    const std::vector<VkPresentModeKHR> presentation_modes{
        VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_MAILBOX_KHR,
        VK_PRESENT_MODE_FIFO_RELAXED_KHR,
        VK_PRESENT_MODE_FIFO_LATEST_READY_KHR};

    EXPECT_EQ(Vulkan::selectPresentationMode(
                  Renderer::PresentationPreference::AllowTearing,
                  presentation_modes, false),
              VK_PRESENT_MODE_MAILBOX_KHR);
}

TEST(
    VulkanSwapchainSelectionTests,
    SelectPresentationModeReturnsFifoForAllowTearingWhenPreferredModesUnavailable) {
    const std::vector<VkPresentModeKHR> presentation_modes{
        VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR,
        VK_PRESENT_MODE_FIFO_LATEST_READY_KHR};

    EXPECT_EQ(Vulkan::selectPresentationMode(
                  Renderer::PresentationPreference::AllowTearing,
                  presentation_modes, false),
              VK_PRESENT_MODE_FIFO_KHR);
}

TEST(VulkanSwapchainSelectionTests,
     SelectPresentationModeReturnsFifoLatestReadyWhenEnabledAndAvailable) {
    const std::vector<VkPresentModeKHR> presentation_modes{
        VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_MAILBOX_KHR,
        VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR,
        VK_PRESENT_MODE_FIFO_LATEST_READY_KHR};

    EXPECT_EQ(Vulkan::selectPresentationMode(
                  Renderer::PresentationPreference::LatestReadyVSync,
                  presentation_modes, true),
              VK_PRESENT_MODE_FIFO_LATEST_READY_KHR);
}

TEST(VulkanSwapchainSelectionTests,
     SelectPresentationModeReturnsMailboxWhenLatestReadyFeatureDisabled) {
    const std::vector<VkPresentModeKHR> presentation_modes{
        VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_MAILBOX_KHR,
        VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR,
        VK_PRESENT_MODE_FIFO_LATEST_READY_KHR};

    EXPECT_EQ(Vulkan::selectPresentationMode(
                  Renderer::PresentationPreference::LatestReadyVSync,
                  presentation_modes, false),
              VK_PRESENT_MODE_MAILBOX_KHR);
}

TEST(VulkanSwapchainSelectionTests,
     SelectPresentationModeReturnsMailboxWhenLatestReadyModeUnavailable) {
    const std::vector<VkPresentModeKHR> presentation_modes{
        VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_MAILBOX_KHR,
        VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR};

    EXPECT_EQ(Vulkan::selectPresentationMode(
                  Renderer::PresentationPreference::LatestReadyVSync,
                  presentation_modes, true),
              VK_PRESENT_MODE_MAILBOX_KHR);
}

TEST(VulkanSwapchainSelectionTests,
     SelectPresentationModeReturnsMailboxForLatestReadyFallback) {
    const std::vector<VkPresentModeKHR> presentation_modes{
        VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_MAILBOX_KHR,
        VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR};

    EXPECT_EQ(Vulkan::selectPresentationMode(
                  Renderer::PresentationPreference::LatestReadyVSync,
                  presentation_modes, false),
              VK_PRESENT_MODE_MAILBOX_KHR);
}

TEST(VulkanSwapchainSelectionTests,
     SelectPresentationModeReturnsFifoForLatestReadyFinalFallback) {
    const std::vector<VkPresentModeKHR> presentation_modes{
        VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR,
        VK_PRESENT_MODE_FIFO_RELAXED_KHR};

    EXPECT_EQ(Vulkan::selectPresentationMode(
                  Renderer::PresentationPreference::LatestReadyVSync,
                  presentation_modes, false),
              VK_PRESENT_MODE_FIFO_KHR);
}
