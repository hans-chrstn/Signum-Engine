#pragma once

#include <cstdint>
namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Reports graphics queue submission and completion progress.
     *
     * Provides a read-only snapshot of the renderer's graphics
     * submission tracking without exposing its synchronization objects.
     *
     * Submission identifiers are monotonically increasing within
     * the renderer's graphics submission stream.
     *
     * A completed submission also establishes completion of earlier
     * submissions in the same stream.
     *
     * @note Does not own Vulkan resources or synchronization objects.
     * @note Completion values reflect confirmed GPU progress and
     * may lag behind actual GPU execution.
     */
    struct VulkanGraphicsSubmissionProgress {
        /**
         * @brief Identifier of the latest recorded graphics submission.
         *
         * Zero indicates that no graphics submission has been recorded.
         */
        std::uint64_t last_submitted{};

        /**
         * @brief Highest graphics submission identifier confirmed complete.
         *
         * All earlier submissions in the same stream are also complete.
         * Zero indicates that no completion has been recorded.
         */
        std::uint64_t last_completed{};
    };
} // namespace SNE::Engine::Renderer::Vulkan
