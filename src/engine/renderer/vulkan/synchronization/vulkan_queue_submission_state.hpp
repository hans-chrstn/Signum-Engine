#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Tracks submission and completion state for one Vulkan queue.
     *
     * Associates monotonically increasing submission identifiers with
     * reusable frame slots and records completion confirmed by their
     * corresponding GPU synchronization operations.
     *
     * Completion of a submission establishes completion of earlier
     * submissions to the same queue.
     *
     * This class maintains CPU-side bookkeeping only. It does not own
     * Vulkan queues, fences, command buffers, or GPU resources.
     *
     * The caller is responsible for synchronizing access, associating
     * submissions with the correct queue, and reporting completion only
     * after the corresponding GPU work has finished.
     *
     * The class is non-copyable and non-movable to preserve one
     * authoritative submission state.
     */
    class VulkanQueueSubmissionState {
      private:
        /**
         * @brief Submission identifiers associated with frame slots.
         *
         * Each entry contains the outstanding submission identifier
         * recorded for its corresponding frame slot.
         *
         * Zero indicates that the slot has no recorded outstanding
         * submission.
         */
        std::vector<std::uint64_t> m_FrameSubmissions;

        /**
         * @brief Identifier of the most recently recorded submission.
         *
         * Starts at zero and increases after each successfully
         * recorded submission.
         */
        std::uint64_t m_LastSubmittedId{};

        /**
         * @brief Highest submission identifier known to be complete.
         *
         * Starts at zero and advances when completion is confirmed
         * for a recorded submission.
         *
         * All earlier identifiers from the same queue are also
         * considered complete.
         */
        std::uint64_t m_LastCompletedId{};

      public:
        /**
         * @brief Initializes submission bookkeeping for frame slots.
         *
         * Allocates one submission identifier entry per frame slot
         * and initializes all identifiers to zero.
         *
         * @param frame_count Number of frame slots to track.
         *
         * @pre frame_count must be greater than zero.
         *
         * @post All frame slots have no recorded outstanding submission.
         * @post The submitted and completed identifiers are zero.
         *
         * @throws std::bad_alloc if storage allocation fails.
         */
        explicit VulkanQueueSubmissionState(std::size_t frame_count);

        /**
         * @brief Destroys the CPU-side submission bookkeeping.
         */
        ~VulkanQueueSubmissionState() noexcept = default;

        VulkanQueueSubmissionState(const VulkanQueueSubmissionState &) = delete;

        auto operator=(const VulkanQueueSubmissionState &)
            -> VulkanQueueSubmissionState & = delete;

        /**
         * @brief Validates whether a frame slot can record a new submission.
         *
         * Checks that the frame index is valid, the frame slot has no
         * outstanding recorded submission, and the submission identifier has
         * not reached its maximum representable value.
         *
         * Performs no state modifications.
         *
         * @param frame_index Index of the frame slot to validate.
         *
         * @pre frame_index must reference an existing frame slot.
         * @pre The frame slot must have no outstanding recorded submission.
         * @pre The latest submission identifier must not be exhausted.
         *
         * @note Call before submitting GPU work to prevent bookkeeping
         * precondition failures after a successful Vulkan queue submission.
         *
         * @note Invalid conditions trigger fatal precondition assertions.
         */
        auto validateCanRecordSubmission(std::size_t frame_index) const -> void;

        /**
         * @brief Records a successful queue submission.
         *
         * Assigns the next submission identifier to the specified
         * frame slot and updates the latest submitted identifier.
         *
         * This function records an already successful submission.
         * It does not submit work to Vulkan.
         *
         * @param frame_index Index of the frame slot used for submission.
         *
         * @return Identifier assigned to the recorded submission.
         *
         * @pre frame_index must reference an existing frame slot.
         * @pre The frame slot must have no outstanding recorded submission.
         * @pre The latest submission identifier must not be at its
         * maximum representable value.
         * @pre The corresponding Vulkan queue submission must have
         * succeeded.
         *
         * @post The frame slot contains the new submission identifier.
         * @post The latest submitted identifier increases by one.
         */
        [[nodiscard]] auto recordSubmission(std::size_t frame_index)
            -> std::uint64_t;

        /**
         * @brief Records confirmed completion of a frame submission.
         *
         * Retrieves the submission identifier associated with the
         * specified frame slot and advances the completed identifier
         * when appropriate.
         *
         * Clears the frame slot's recorded submission identifier.
         *
         * If the frame slot has no recorded submission, no completion
         * state is changed.
         *
         * @param frame_index Index of the completed frame slot.
         *
         * @pre frame_index must reference an existing frame slot.
         * @pre If the slot contains a submission, completion of its
         * associated GPU work must have been confirmed.
         *
         * @post The frame slot contains no outstanding submission.
         * @post The latest completed identifier never decreases.
         */
        auto recordCompletion(std::size_t frame_index) -> void;

        /**
         * @brief Returns the latest recorded submission identifier.
         *
         * @return Latest submission identifier, or zero if no
         * submission has been recorded.
         */
        [[nodiscard]] auto lastSubmitted() const noexcept -> std::uint64_t;

        /**
         * @brief Checks whether a submission is known to be complete.
         *
         * A submission is complete when its identifier is no greater
         * than the highest confirmed completed identifier.
         *
         * Identifier zero represents no submitted work and is
         * considered complete.
         *
         * @param submission_id Submission identifier to check.
         *
         * @return true if the submission is known to be complete;
         * otherwise false.
         */
        [[nodiscard]] auto
        isComplete(std::uint64_t submission_id) const noexcept -> bool;
    };
} // namespace SNE::Engine::Renderer::Vulkan
