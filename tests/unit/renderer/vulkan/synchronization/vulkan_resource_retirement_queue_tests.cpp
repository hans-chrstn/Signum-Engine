#include "engine/renderer/vulkan/synchronization/vulkan_queue_submission_state.hpp"
#include "engine/renderer/vulkan/synchronization/vulkan_resource_retirement_queue.hpp"
#include <cstddef>
#include <gtest/gtest.h>
#include <memory>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    struct CountingDeleter {
        int *destruction_count{};

        auto operator()(const int *value) const noexcept -> void {
            ++(*destruction_count);
            delete value;
        }
    };

    using TrackedResource = std::unique_ptr<int, CountingDeleter>;

    [[nodiscard]] auto makeTrackedResource(int &destruction_count)
        -> TrackedResource {
        return TrackedResource{
            new int{},
            CountingDeleter{&destruction_count},
        };
    }
} // namespace

TEST(VulkanResourceRetirementQueueTests, EmptyQueueCanBeCollected) {
    Vulkan::VulkanQueueSubmissionState submissions{1U};
    Vulkan::VulkanResourceRetirementQueue queue;

    queue.collectCompleted(submissions);
}

TEST(VulkanResourceRetirementQueueTests,
     RetainsResourceUntilSubmissionCompletes) {
    int destruction_count{};

    Vulkan::VulkanQueueSubmissionState submissions{1U};
    Vulkan::VulkanResourceRetirementQueue queue;

    auto resource = makeTrackedResource(destruction_count);
    const auto submission_id = submissions.recordSubmission(0U);

    queue.retire(submission_id, resource);

    EXPECT_EQ(resource.get(), nullptr);
    EXPECT_EQ(destruction_count, 0);

    queue.collectCompleted(submissions);

    EXPECT_EQ(destruction_count, 0);

    submissions.recordCompletion(0U);
    queue.collectCompleted(submissions);

    EXPECT_EQ(destruction_count, 1);
}

TEST(VulkanResourceRetirementQueueTests,
     CollectsCompletedResourcesWithoutReleasingIncompleteResources) {
    int first_destructions{};
    int second_destructions{};

    Vulkan::VulkanQueueSubmissionState submissions{2U};
    Vulkan::VulkanResourceRetirementQueue queue;

    auto first_resource = makeTrackedResource(first_destructions);
    auto second_resource = makeTrackedResource(second_destructions);

    const auto first_id = submissions.recordSubmission(0U);
    const auto second_id = submissions.recordSubmission(1U);

    queue.retire(first_id, first_resource);
    queue.retire(second_id, second_resource);

    submissions.recordCompletion(0U);
    queue.collectCompleted(submissions);

    EXPECT_EQ(first_destructions, 1);
    EXPECT_EQ(second_destructions, 0);

    submissions.recordCompletion(1U);
    queue.collectCompleted(submissions);

    EXPECT_EQ(first_destructions, 1);
    EXPECT_EQ(second_destructions, 1);
}

TEST(VulkanResourceRetirementQueueTests,
     LaterSubmissionCompletionReleasesEarlierResources) {
    int first_destructions{};
    int second_destructions{};

    Vulkan::VulkanQueueSubmissionState submissions{2U};
    Vulkan::VulkanResourceRetirementQueue queue;

    auto first_resource = makeTrackedResource(first_destructions);
    auto second_resource = makeTrackedResource(second_destructions);

    const auto first_id = submissions.recordSubmission(0U);
    const auto second_id = submissions.recordSubmission(1U);

    queue.retire(first_id, first_resource);
    queue.retire(second_id, second_resource);

    submissions.recordCompletion(1U);
    queue.collectCompleted(submissions);

    EXPECT_EQ(first_destructions, 1);
    EXPECT_EQ(second_destructions, 1);
}

TEST(VulkanResourceRetirementQueueTests,
     RepeatedCollectionDoesNotReleaseResourcesTwice) {
    int destruction_count{};

    Vulkan::VulkanQueueSubmissionState submissions{1U};
    Vulkan::VulkanResourceRetirementQueue queue;

    auto resource = makeTrackedResource(destruction_count);
    const auto submission_id = submissions.recordSubmission(0U);

    queue.retire(submission_id, resource);

    submissions.recordCompletion(0U);

    queue.collectCompleted(submissions);
    queue.collectCompleted(submissions);

    EXPECT_EQ(destruction_count, 1);
}

TEST(VulkanResourceRetirementQueueTests,
     VectorGrowthPreservesPendingResourceOwnership) {
    constexpr std::size_t resource_count = 32U;

    int destruction_count{};

    Vulkan::VulkanQueueSubmissionState submissions{resource_count};
    Vulkan::VulkanResourceRetirementQueue queue;

    for (std::size_t i{}; i < resource_count; ++i) {
        auto resource = makeTrackedResource(destruction_count);
        const auto submission_id = submissions.recordSubmission(i);

        queue.retire(submission_id, resource);

        EXPECT_EQ(resource.get(), nullptr);
    }

    EXPECT_EQ(destruction_count, 0);

    submissions.recordCompletion(resource_count - 1U);
    queue.collectCompleted(submissions);

    EXPECT_EQ(destruction_count, static_cast<int>(resource_count));
}

TEST(VulkanResourceRetirementQueueTests,
     RejectsDestructionWithPendingResources) {
    ASSERT_DEATH(
        {
            int destruction_count{};

            Vulkan::VulkanQueueSubmissionState submissions{1U};
            Vulkan::VulkanResourceRetirementQueue queue;

            auto resource = makeTrackedResource(destruction_count);
            const auto submission_id = submissions.recordSubmission(0U);

            queue.retire(submission_id, resource);
        },
        "must be empty before destruction");
}
