#include "engine/renderer/vulkan/presentation/vulkan_swapchain_result_policy.hpp"
#include <gtest/gtest.h>
#include <vulkan/vulkan.h>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

TEST(VulkanSwapchainResultPolicyTests,
     SelectAcquireActionContinuesFrameForSuccess) {
    EXPECT_EQ(Vulkan::selectSwapchainAcquireAction(VK_SUCCESS),
              Vulkan::SwapchainAcquireAction::ContinueFrame);
}

TEST(VulkanSwapchainResultPolicyTests,
     SelectAcquireActionContinuesAndRecreatesForSuboptimal) {
    EXPECT_EQ(
        Vulkan::selectSwapchainAcquireAction(VK_SUBOPTIMAL_KHR),
        Vulkan::SwapchainAcquireAction::ContinueFrameAndRecreateAfterPresent);
}

TEST(VulkanSwapchainResultPolicyTests,
     SelectAcquireActionReturnsWithoutAdvancingForTimeout) {
    EXPECT_EQ(Vulkan::selectSwapchainAcquireAction(VK_TIMEOUT),
              Vulkan::SwapchainAcquireAction::ReturnWithoutAdvancingFrame);
}

TEST(VulkanSwapchainResultPolicyTests,
     SelectAcquireActionReturnsWithoutAdvancingForNotReady) {
    EXPECT_EQ(Vulkan::selectSwapchainAcquireAction(VK_NOT_READY),
              Vulkan::SwapchainAcquireAction::ReturnWithoutAdvancingFrame);
}

TEST(VulkanSwapchainResultPolicyTests,
     SelectAcquireActionRecreatesAndReturnsForOutOfDate) {
    EXPECT_EQ(
        Vulkan::selectSwapchainAcquireAction(VK_ERROR_OUT_OF_DATE_KHR),
        Vulkan::SwapchainAcquireAction::RecreateAndReturnWithoutAdvancingFrame);
}

TEST(VulkanSwapchainResultPolicyTests,
     SelectAcquireActionReportsUnexpectedResultAsFailure) {
    EXPECT_EQ(Vulkan::selectSwapchainAcquireAction(VK_ERROR_DEVICE_LOST),
              Vulkan::SwapchainAcquireAction::Failure);
}

TEST(VulkanSwapchainResultPolicyTests,
     SelectPresentActionAdvancesFrameForSuccess) {
    EXPECT_EQ(Vulkan::selectSwapchainPresentAction(VK_SUCCESS),
              Vulkan::SwapchainPresentAction::AdvanceFrame);
}

TEST(VulkanSwapchainResultPolicyTests,
     SelectPresentActionRecreatesAndAdvancesForSuboptimal) {
    EXPECT_EQ(Vulkan::selectSwapchainPresentAction(VK_SUBOPTIMAL_KHR),
              Vulkan::SwapchainPresentAction::RecreateAndAdvanceFrame);
}

TEST(VulkanSwapchainResultPolicyTests,
     SelectPresentActionRecreatesAndReturnsForOutOfDate) {
    EXPECT_EQ(
        Vulkan::selectSwapchainPresentAction(VK_ERROR_OUT_OF_DATE_KHR),
        Vulkan::SwapchainPresentAction::RecreateAndReturnWithoutAdvancingFrame);
}

TEST(VulkanSwapchainResultPolicyTests,
     SelectPresentActionReportsUnexpectedResultAsFailure) {
    EXPECT_EQ(Vulkan::selectSwapchainPresentAction(VK_ERROR_DEVICE_LOST),
              Vulkan::SwapchainPresentAction::Failure);
}
