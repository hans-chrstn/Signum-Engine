#pragma once

#include <cstdint>

namespace SNE::Engine::Renderer {
    /**
     * @brief Reports graphics submission and completion progress.
     *
     * Provides a backend-independent, read-only snapshot of the
     * renderer's graphics submission tracking without exposing
     * backend synchronization objects.
     *
     * Submission identifiers are monotonically increasing within
     * the renderer's graphics submission stream.
     *
     * A completed submission also establishes completion of earlier
     * submissions in the same stream.
     *
     * @note This structure does not own GPU resources or
     * synchronization objects.
     *
     * @note Completion values reflect confirmed GPU progress and
     * may lag behind actual GPU execution.
     *
     * @note Submission identifiers from different renderers or
     * independent submission streams are not interchangeable.
     */
    struct GraphicsSubmissionProgress {
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
         *
         * Zero indicates that no completion has been recorded.
         */
        std::uint64_t last_completed{};
    };
} // namespace SNE::Engine::Renderer
