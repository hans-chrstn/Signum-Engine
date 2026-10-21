#include "vulkan_queue_submission_state.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace SNE::Engine::Renderer::Vulkan {
    VulkanQueueSubmissionState::VulkanQueueSubmissionState(
        std::size_t frame_count) {
        if (frame_count == 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanQueueSubmissionState requires at least one frame slot");
        }

        m_FrameSubmissions.resize(frame_count);
    }

    auto VulkanQueueSubmissionState::validateCanRecordSubmission(
        std::size_t frame_index) const -> void {
        if (frame_index >= m_FrameSubmissions.size()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanQueueSubmissionState frame index is out of range");
        }

        if (m_FrameSubmissions[frame_index] != 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanQueueSubmissionState frame slot has an outstanding "
                "submission");
        }

        if (m_LastSubmittedId == std::numeric_limits<std::uint64_t>::max()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanQueueSubmissionState submission identifier exhausted");
        }
    }

    auto VulkanQueueSubmissionState::recordSubmission(std::size_t frame_index)
        -> std::uint64_t {
        validateCanRecordSubmission(frame_index);

        ++m_LastSubmittedId;

        m_FrameSubmissions[frame_index] = m_LastSubmittedId;

        return m_LastSubmittedId;
    }

    auto VulkanQueueSubmissionState::recordCompletion(std::size_t frame_index)
        -> void {
        if (frame_index >= m_FrameSubmissions.size()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanQueueSubmissionState completion frame index is out of "
                "range");
        }

        const std::uint64_t recorded_id = m_FrameSubmissions[frame_index];

        if (recorded_id == 0U) {
            return;
        }

        m_LastCompletedId = std::max(recorded_id, m_LastCompletedId);

        m_FrameSubmissions[frame_index] = 0U;
    }

    auto VulkanQueueSubmissionState::lastSubmitted() const noexcept
        -> std::uint64_t {
        return m_LastSubmittedId;
    }

    auto VulkanQueueSubmissionState::lastCompleted() const noexcept
        -> std::uint64_t {
        return m_LastCompletedId;
    }

    auto VulkanQueueSubmissionState::isComplete(
        std::uint64_t submission_id) const noexcept -> bool {
        return submission_id <= m_LastCompletedId;
    }
} // namespace SNE::Engine::Renderer::Vulkan
