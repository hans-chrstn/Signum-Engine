#include "../vulkan_integration_test_fixture.hpp"
#include <cstdint>
#include <gtest/gtest.h>
#include <stdexcept>
#include <vulkan/vulkan.h>

TEST_F(VulkanIntegrationTest, ExecutesImmediateSubmissionCallback) {
    bool callback_called{false};

    immediateSubmission().execute(
        [&](VkCommandBuffer) -> void { callback_called = true; });

    EXPECT_TRUE(callback_called);
}

TEST_F(VulkanIntegrationTest, ReusesImmediateSubmissionAcrossExecutions) {
    std::uint32_t execution_count{};

    immediateSubmission().execute(
        [&](VkCommandBuffer) -> void { ++execution_count; });

    immediateSubmission().execute(
        [&](VkCommandBuffer) -> void { ++execution_count; });

    EXPECT_EQ(execution_count, 2U);
}

TEST_F(VulkanIntegrationTest, RecoversAfterRecordingCallbackThrows) {
    bool recording_failure_observed{false};

    try {
        immediateSubmission().execute([](VkCommandBuffer) -> void {
            throw std::runtime_error("Recording callback failure");
        });
    } catch (const std::runtime_error &) {
        recording_failure_observed = true;
    }

    EXPECT_TRUE(recording_failure_observed);

    bool callback_called{false};

    immediateSubmission().execute(
        [&](VkCommandBuffer) -> void { callback_called = true; });

    EXPECT_TRUE(callback_called);
}
