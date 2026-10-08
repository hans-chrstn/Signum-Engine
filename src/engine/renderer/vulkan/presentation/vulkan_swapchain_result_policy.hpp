#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Defines the renderer action selected from a swapchain image
     * acquisition result.
     */
    enum class SwapchainAcquireAction : std::uint8_t {
        /**
         * Continue processing the current frame normally.
         */
        ContinueFrame,

        /**
         * Continue processing the current frame and recreate
         * swapchain-dependent resources after presentation.
         */
        ContinueFrameAndRecreateAfterPresent,

        /**
         * Stop processing the current frame without advancing the
         * frame-in-flight index.
         */
        ReturnWithoutAdvancingFrame,

        /**
         * Recreate swapchain-dependent resources and stop processing the
         * current frame without advancing the frame-in-flight index.
         */
        RecreateAndReturnWithoutAdvancingFrame,

        /**
         * Treat the acquisition result as a runtime failure.
         */
        Failure,
    };

    /**
     * @brief Defines the renderer action selected from a swapchain presentation
     * result.
     */
    enum class SwapchainPresentAction : std::uint8_t {
        /**
         * Complete the current frame and advance the frame-in-flight index.
         */
        AdvanceFrame,

        /**
         * Recreate swapchain-dependent resources before completing and
         * advancing the current frame.
         */
        RecreateAndAdvanceFrame,

        /**
         * Recreate swapchain-dependent resources and stop processing the
         * current frame without advancing the frame-in-flight index.
         */
        RecreateAndReturnWithoutAdvancingFrame,

        /**
         * Treat the presentation result as a runtime failure.
         */
        Failure,
    };

    /**
     * @brief Selects the renderer action for a Vulkan swapchain image
     * acquisition result.
     *
     * Successful acquisition allows frame processing to continue.
     * Suboptimal acquisition allows the acquired image to be used while
     * requesting swapchain recreation after presentation.
     *
     * Temporary acquisition conditions return without consuming the current
     * frame-in-flight slot. An out-of-date swapchain requires recreation before
     * returning.
     *
     * Results not explicitly handled by renderer presentation policy are
     * classified as runtime failures.
     *
     * @param result Result returned by vkAcquireNextImageKHR.
     *
     * @return Renderer action selected for the acquisition result.
     */
    [[nodiscard]] auto selectSwapchainAcquireAction(VkResult result) noexcept
        -> SwapchainAcquireAction;

    /**
     * @brief Selects the renderer action for a Vulkan swapchain presentation
     * result.
     *
     * Successful presentation completes the frame normally. Suboptimal
     * presentation requests swapchain recreation while still completing the
     * submitted frame.
     *
     * An out-of-date swapchain requires recreation before returning without
     * advancing the current frame-in-flight slot.
     *
     * Results not explicitly handled by renderer presentation policy are
     * classified as runtime failures.
     *
     * @param result Result returned by vkQueuePresentKHR.
     *
     * @return Renderer action selected for the presentation result.
     */
    [[nodiscard]] auto selectSwapchainPresentAction(VkResult result) noexcept
        -> SwapchainPresentAction;
} // namespace SNE::Engine::Renderer::Vulkan
