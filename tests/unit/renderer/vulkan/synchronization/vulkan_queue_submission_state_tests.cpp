#include "engine/renderer/vulkan/synchronization/vulkan_queue_submission_state.hpp"
#include <cstdint>
#include <gtest/gtest.h>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

TEST(VulkanQueueSubmissionStateTests, InitializesWithNoSubmissions) {
    const Vulkan::VulkanQueueSubmissionState state{2U};

    EXPECT_EQ(state.lastSubmitted(), 0U);
    EXPECT_TRUE(state.isComplete(0U));
    EXPECT_FALSE(state.isComplete(1U));
}

TEST(VulkanQueueSubmissionStateTests, AssignsIncreasingSubmissionIds) {
    Vulkan::VulkanQueueSubmissionState state{2U};

    EXPECT_EQ(state.recordSubmission(0U), 1U);
    EXPECT_EQ(state.recordSubmission(1U), 2U);

    EXPECT_EQ(state.lastSubmitted(), 2U);
    EXPECT_FALSE(state.isComplete(1U));
    EXPECT_FALSE(state.isComplete(2U));
    EXPECT_FALSE(state.isComplete(3U));
}

TEST(VulkanQueueSubmissionStateTests, CompletionMarksSubmissionComplete) {
    Vulkan::VulkanQueueSubmissionState state{1U};

    const std::uint64_t submission_id = state.recordSubmission(0U);

    EXPECT_FALSE(state.isComplete(submission_id));

    state.recordCompletion(0U);

    EXPECT_TRUE(state.isComplete(submission_id));
    EXPECT_EQ(state.lastSubmitted(), submission_id);
}

TEST(VulkanQueueSubmissionStateTests,
     LaterQueueCompletionAlsoCompletesEarlierSubmissions) {
    Vulkan::VulkanQueueSubmissionState state{3U};

    const std::uint64_t first = state.recordSubmission(0U);
    const std::uint64_t second = state.recordSubmission(1U);
    const std::uint64_t third = state.recordSubmission(2U);

    state.recordCompletion(2U);

    EXPECT_TRUE(state.isComplete(first));
    EXPECT_TRUE(state.isComplete(second));
    EXPECT_TRUE(state.isComplete(third));
    EXPECT_FALSE(state.isComplete(third + 1U));
}

TEST(VulkanQueueSubmissionStateTests,
     EarlierCompletionDoesNotDecreaseCompletedId) {
    Vulkan::VulkanQueueSubmissionState state{2U};

    const std::uint64_t first = state.recordSubmission(0U);
    const std::uint64_t second = state.recordSubmission(1U);

    state.recordCompletion(1U);
    state.recordCompletion(0U);

    EXPECT_TRUE(state.isComplete(first));
    EXPECT_TRUE(state.isComplete(second));
    EXPECT_FALSE(state.isComplete(second + 1U));
}

TEST(VulkanQueueSubmissionStateTests, CompletedFrameSlotCanBeReused) {
    Vulkan::VulkanQueueSubmissionState state{1U};

    const std::uint64_t first = state.recordSubmission(0U);

    state.recordCompletion(0U);

    const std::uint64_t second = state.recordSubmission(0U);

    EXPECT_EQ(second, first + 1U);
    EXPECT_TRUE(state.isComplete(first));
    EXPECT_FALSE(state.isComplete(second));
}

TEST(VulkanQueueSubmissionStateTests, EmptyFrameCompletionDoesNotChangeState) {
    Vulkan::VulkanQueueSubmissionState state{2U};

    const std::uint64_t submission_id = state.recordSubmission(0U);

    state.recordCompletion(1U);

    EXPECT_EQ(state.lastSubmitted(), submission_id);
    EXPECT_FALSE(state.isComplete(submission_id));

    state.recordCompletion(0U);
    state.recordCompletion(0U);

    EXPECT_TRUE(state.isComplete(submission_id));
}

TEST(VulkanQueueSubmissionStateTests,
     FrameSlotsRemainOccupiedUntilCompletionIsRecorded) {
    Vulkan::VulkanQueueSubmissionState state{2U};

    static_cast<void>(state.recordSubmission(0U));
    static_cast<void>(state.recordSubmission(1U));

    state.recordCompletion(1U);

    ASSERT_DEATH(static_cast<void>(state.recordSubmission(0U)),
                 "frame slot has an outstanding submission");
}

TEST(VulkanQueueSubmissionStateTests, RejectsZeroFrameSlots) {
    ASSERT_DEATH(Vulkan::VulkanQueueSubmissionState state{0U},
                 "requires at least one frame slot");
}

TEST(VulkanQueueSubmissionStateTests, RejectsInvalidSubmissionFrameIndex) {
    Vulkan::VulkanQueueSubmissionState state{2U};

    ASSERT_DEATH(static_cast<void>(state.recordSubmission(2U)),
                 "frame index is out of range");
}

TEST(VulkanQueueSubmissionStateTests, RejectsInvalidCompletionFrameIndex) {
    Vulkan::VulkanQueueSubmissionState state{2U};

    ASSERT_DEATH(state.recordCompletion(2U), "frame index is out of range");
}

TEST(VulkanQueueSubmissionStateTests,
     IndependentInstancesMaintainIndependentState) {
    Vulkan::VulkanQueueSubmissionState first{1U};
    Vulkan::VulkanQueueSubmissionState second{1U};

    const std::uint64_t first_id = first.recordSubmission(0U);
    const std::uint64_t second_id = second.recordSubmission(0U);

    first.recordCompletion(0U);

    EXPECT_TRUE(first.isComplete(first_id));
    EXPECT_FALSE(second.isComplete(second_id));
}

TEST(VulkanQueueSubmissionStateTests, ValidatesAvailableFrameSlot) {
    Vulkan::VulkanQueueSubmissionState state{2U};

    state.validateCanRecordSubmission(0U);
    state.validateCanRecordSubmission(1U);

    EXPECT_EQ(state.lastSubmitted(), 0U);
    EXPECT_FALSE(state.isComplete(1U));
}

TEST(VulkanQueueSubmissionStateTests, RejectsValidationOfOccupiedFrameSlot) {
    Vulkan::VulkanQueueSubmissionState state{2U};

    const std::uint64_t submission_id = state.recordSubmission(0U);

    ASSERT_DEATH(state.validateCanRecordSubmission(0U),
                 "frame slot has an outstanding submission");

    EXPECT_FALSE(state.isComplete(submission_id));
}

TEST(VulkanQueueSubmissionStateTests, RejectsValidationOfInvalidFrameIndex) {
    Vulkan::VulkanQueueSubmissionState state{2U};

    ASSERT_DEATH(state.validateCanRecordSubmission(2U),
                 "frame index is out of range");
}

TEST(VulkanQueueSubmissionStateTests, AllowsValidationAfterRecordedCompletion) {
    Vulkan::VulkanQueueSubmissionState state{1U};

    const std::uint64_t first = state.recordSubmission(0U);

    state.recordCompletion(0U);

    state.validateCanRecordSubmission(0U);

    const std::uint64_t second = state.recordSubmission(0U);

    EXPECT_EQ(second, first + 1U);
    EXPECT_TRUE(state.isComplete(first));
    EXPECT_FALSE(state.isComplete(second));
}
