#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/vulkan/vulkan_swapchain_selection.hpp"
#include <cstdint>
#include <gtest/gtest.h>
#include <vector>
#include <vulkan/vulkan.h>

namespace Platform = SNE::Engine::Platform;
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

TEST(VulkanSwapchainSelectionTests,
     SelectSwapExtentReturnsCurrentExtentWhenSurfaceSpecifiesExtent) {
    constexpr VkExtent2D expected_extent{
        .width = 1280U,
        .height = 720U,
    };

    VkSurfaceCapabilitiesKHR capabilities{};
    capabilities.currentExtent = expected_extent;

    constexpr Platform::FramebufferSize framebuffer_size{
        .width = 1920,
        .height = 1080,
    };

    const VkExtent2D selected_extent =
        Vulkan::selectSwapExtent(capabilities, framebuffer_size);

    EXPECT_EQ(selected_extent.width, expected_extent.width);
    EXPECT_EQ(selected_extent.height, expected_extent.height);
}

TEST(VulkanSwapchainSelectionTests,
     SelectSwapExtentReturnsFramebufferExtentWhenWithinSupportedRange) {
    constexpr VkExtent2D undefined_current_extent{
        .width = UINT32_MAX,
        .height = UINT32_MAX,
    };

    constexpr VkExtent2D minimum_extent{
        .width = 640U,
        .height = 480U,
    };

    constexpr VkExtent2D maximum_extent{
        .width = 1920U,
        .height = 1080U,
    };

    VkSurfaceCapabilitiesKHR capabilities{};
    capabilities.currentExtent = undefined_current_extent;
    capabilities.minImageExtent = minimum_extent;
    capabilities.maxImageExtent = maximum_extent;

    constexpr Platform::FramebufferSize framebuffer_size{
        .width = 1280,
        .height = 800,
    };

    const VkExtent2D selected_extent =
        Vulkan::selectSwapExtent(capabilities, framebuffer_size);

    EXPECT_EQ(selected_extent.width,
              static_cast<std::uint32_t>(framebuffer_size.width));
    EXPECT_EQ(selected_extent.height,
              static_cast<std::uint32_t>(framebuffer_size.height));
}

TEST(VulkanSwapchainSelectionTests,
     SelectSwapExtentClampsFramebufferExtentToMinimum) {
    constexpr VkExtent2D undefined_current_extent{
        .width = UINT32_MAX,
        .height = UINT32_MAX,
    };

    constexpr VkExtent2D minimum_extent{
        .width = 640U,
        .height = 480U,
    };

    constexpr VkExtent2D maximum_extent{
        .width = 1920U,
        .height = 1080U,
    };

    VkSurfaceCapabilitiesKHR capabilities{};
    capabilities.currentExtent = undefined_current_extent;
    capabilities.minImageExtent = minimum_extent;
    capabilities.maxImageExtent = maximum_extent;

    constexpr Platform::FramebufferSize framebuffer_size{
        .width = 480,
        .height = 360,
    };

    const VkExtent2D selected_extent =
        Vulkan::selectSwapExtent(capabilities, framebuffer_size);

    EXPECT_EQ(selected_extent.width, minimum_extent.width);
    EXPECT_EQ(selected_extent.height, minimum_extent.height);
}

TEST(VulkanSwapchainSelectionTests,
     SelectSwapExtentClampsFramebufferExtentToMaximum) {
    constexpr VkExtent2D undefined_current_extent{
        .width = UINT32_MAX,
        .height = UINT32_MAX,
    };

    constexpr VkExtent2D minimum_extent{
        .width = 640U,
        .height = 480U,
    };

    constexpr VkExtent2D maximum_extent{
        .width = 1920U,
        .height = 1080U,
    };

    VkSurfaceCapabilitiesKHR capabilities{};
    capabilities.currentExtent = undefined_current_extent;
    capabilities.minImageExtent = minimum_extent;
    capabilities.maxImageExtent = maximum_extent;

    constexpr Platform::FramebufferSize framebuffer_size{
        .width = 2560,
        .height = 1440,
    };

    const VkExtent2D selected_extent =
        Vulkan::selectSwapExtent(capabilities, framebuffer_size);

    EXPECT_EQ(selected_extent.width, maximum_extent.width);
    EXPECT_EQ(selected_extent.height, maximum_extent.height);
}

TEST(VulkanSwapchainSelectionTests,
     SelectSwapExtentClampsWidthAndHeightIndependently) {
    constexpr VkExtent2D undefined_current_extent{
        .width = UINT32_MAX,
        .height = UINT32_MAX,
    };

    constexpr VkExtent2D minimum_extent{
        .width = 640U,
        .height = 480U,
    };

    constexpr VkExtent2D maximum_extent{
        .width = 1920U,
        .height = 1080U,
    };

    VkSurfaceCapabilitiesKHR capabilities{};
    capabilities.currentExtent = undefined_current_extent;
    capabilities.minImageExtent = minimum_extent;
    capabilities.maxImageExtent = maximum_extent;

    constexpr Platform::FramebufferSize framebuffer_size{
        .width = 2560,
        .height = 360,
    };

    const VkExtent2D selected_extent =
        Vulkan::selectSwapExtent(capabilities, framebuffer_size);

    EXPECT_EQ(selected_extent.width, maximum_extent.width);
    EXPECT_EQ(selected_extent.height, minimum_extent.height);
}

TEST(VulkanSwapchainSelectionTests,
     SelectSwapchainImageCountReturnsMinimumPlusOneWhenSupported) {
    VkSurfaceCapabilitiesKHR capabilities{};
    capabilities.minImageCount = 2U;
    capabilities.maxImageCount = 3U;

    const std::uint32_t selected =
        Vulkan::selectSwapchainImageCount(capabilities);
    EXPECT_EQ(selected, capabilities.minImageCount + 1U);
}

TEST(VulkanSwapchainSelectionTests,
     SelectSwapchainImageCountClampsToMaximumWhenPreferredCountExceedsMaximum) {
    VkSurfaceCapabilitiesKHR capabilities{};
    capabilities.minImageCount = 2U;
    capabilities.maxImageCount = 2U;

    const std::uint32_t selected =
        Vulkan::selectSwapchainImageCount(capabilities);
    EXPECT_EQ(selected, capabilities.maxImageCount);
}

TEST(VulkanSwapchainSelectionTests,
     SelectSwapchainImageCountReturnsMinimumPlusOneWhenMaximumIsUnbounded) {
    VkSurfaceCapabilitiesKHR capabilities{};
    capabilities.minImageCount = 3U;
    capabilities.maxImageCount = 0U;

    const std::uint32_t selected =
        Vulkan::selectSwapchainImageCount(capabilities);
    EXPECT_EQ(selected, capabilities.minImageCount + 1U);
}

TEST(VulkanSwapchainSelectionTests,
     SelectCompositeAlphaReturnsOpaqueWhenSupported) {
    VkSurfaceCapabilitiesKHR capabilities{};
    capabilities.supportedCompositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    EXPECT_EQ(Vulkan::selectCompositeAlpha(capabilities),
              VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);
}

TEST(VulkanSwapchainSelectionTests,
     SelectCompositeAlphaReturnsPreMultipliedWhenOpaqueUnavailable) {
    VkSurfaceCapabilitiesKHR capabilities{};
    capabilities.supportedCompositeAlpha =
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;

    EXPECT_EQ(Vulkan::selectCompositeAlpha(capabilities),
              VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR);
}

TEST(
    VulkanSwapchainSelectionTests,
    SelectCompositeAlphaReturnsPostMultipliedWhenHigherPriorityModesUnavailable) {
    VkSurfaceCapabilitiesKHR capabilities{};
    capabilities.supportedCompositeAlpha =
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;

    EXPECT_EQ(Vulkan::selectCompositeAlpha(capabilities),
              VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR);
}

TEST(VulkanSwapchainSelectionTests,
     SelectCompositeAlphaReturnsInheritWhenOnlyInheritSupported) {
    VkSurfaceCapabilitiesKHR capabilities{};
    capabilities.supportedCompositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;

    EXPECT_EQ(Vulkan::selectCompositeAlpha(capabilities),
              VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR);
}

TEST(VulkanSwapchainSelectionTests,
     SelectCompositeAlphaFailsWhenNoRecognizedModeIsSupported) {
    VkSurfaceCapabilitiesKHR capabilities{};
    capabilities.supportedCompositeAlpha = 0U;

    ASSERT_DEATH(
        static_cast<void>(Vulkan::selectCompositeAlpha(capabilities)),
        "VulkanSwapchain could not select a supported composite-alpha mode");
}
