#include "vulkan_swapchain_result_policy.hpp"

namespace SNE::Engine::Renderer::Vulkan {
    auto selectSwapchainAcquireAction(VkResult result) noexcept
        -> SwapchainAcquireAction {
        switch (result) {
        case VK_SUCCESS:
            return SwapchainAcquireAction::ContinueFrame;

        case VK_SUBOPTIMAL_KHR:
            return SwapchainAcquireAction::ContinueFrameAndRecreateAfterPresent;

        case VK_TIMEOUT:
        case VK_NOT_READY:
            return SwapchainAcquireAction::ReturnWithoutAdvancingFrame;

        case VK_ERROR_OUT_OF_DATE_KHR:
            return SwapchainAcquireAction::
                RecreateAndReturnWithoutAdvancingFrame;

        default:
            return SwapchainAcquireAction::Failure;
        }
    }

    auto selectSwapchainPresentAction(VkResult result) noexcept
        -> SwapchainPresentAction {
        switch (result) {
        case VK_SUCCESS:
            return SwapchainPresentAction::AdvanceFrame;

        case VK_SUBOPTIMAL_KHR:
            return SwapchainPresentAction::RecreateAndAdvanceFrame;

        case VK_ERROR_OUT_OF_DATE_KHR:
            return SwapchainPresentAction::
                RecreateAndReturnWithoutAdvancingFrame;

        default:
            return SwapchainPresentAction::Failure;
        }
    }
} // namespace SNE::Engine::Renderer::Vulkan
