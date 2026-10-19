
#include "engine/renderer/vulkan/synchronization/vulkan_queue_submission_state.hpp"
#include "engine/renderer/vulkan/synchronization/vulkan_resource_retirement_queue.hpp"
#include <array>
#include <cstddef>
#include <gtest/gtest.h>
#include <memory>
#include <optional>
#include <utility>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    struct OrderRecordingDeleter {
        std::array<int, 2> *destruction_order{};
        std::size_t *next_index{};
        int resource_id{};

        template <typename T>
        auto operator()(T *resource) const noexcept -> void {
            (*destruction_order)[(*next_index)++] = resource_id;
            delete resource;
        }
    };

    struct TripleOrderRecordingDeleter {
        std::array<int, 3> *destruction_order{};
        std::size_t *next_index{};
        int resource_id{};

        template <typename T>
        auto operator()(T *resource) const noexcept -> void {
            (*destruction_order)[(*next_index)++] = resource_id;
            delete resource;
        }
    };

    struct CountingDeleter {
        int *destruction_count{};

        template <typename T> auto operator()(T *value) const noexcept -> void {
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

TEST(VulkanResourceRetirementQueueTests,
     ReplacingActiveResourceRetainsOldUntilCompletion) {
    int old_destructions{};
    int replacement_destructions{};

    Vulkan::VulkanQueueSubmissionState submissions{1U};
    Vulkan::VulkanResourceRetirementQueue queue;

    std::optional<TrackedResource> active_resource{
        std::in_place, makeTrackedResource(old_destructions)};

    auto replacement = makeTrackedResource(replacement_destructions);

    const auto submission_id = submissions.recordSubmission(0U);

    queue.retire(submission_id, active_resource.value());

    EXPECT_TRUE(active_resource.has_value());
    EXPECT_EQ(active_resource->get(), nullptr);

    active_resource.emplace(std::move(replacement));

    EXPECT_EQ(replacement.get(), nullptr);
    EXPECT_NE(active_resource->get(), nullptr);

    EXPECT_EQ(old_destructions, 0);
    EXPECT_EQ(replacement_destructions, 0);

    queue.collectCompleted(submissions);

    EXPECT_EQ(old_destructions, 0);
    EXPECT_EQ(replacement_destructions, 0);

    submissions.recordCompletion(0U);
    queue.collectCompleted(submissions);

    EXPECT_EQ(old_destructions, 1);
    EXPECT_EQ(replacement_destructions, 0);

    active_resource.reset();

    EXPECT_EQ(old_destructions, 1);
    EXPECT_EQ(replacement_destructions, 1);
}

TEST(VulkanResourceRetirementQueueTests, RetiresTwoResourcesWithOneSubmission) {
    int first_destructions{};
    int second_destructions{};

    Vulkan::VulkanQueueSubmissionState submissions{1U};
    Vulkan::VulkanResourceRetirementQueue queue;

    auto first = makeTrackedResource(first_destructions);
    auto second = makeTrackedResource(second_destructions);

    const auto submission_id = submissions.recordSubmission(0U);

    queue.retire(submission_id, first, second);

    EXPECT_EQ(first.get(), nullptr);
    EXPECT_EQ(second.get(), nullptr);

    EXPECT_EQ(first_destructions, 0);
    EXPECT_EQ(second_destructions, 0);

    submissions.recordCompletion(0U);
    queue.collectCompleted(submissions);

    EXPECT_EQ(first_destructions, 1);
    EXPECT_EQ(second_destructions, 1);
}

TEST(VulkanResourceRetirementQueueTests,
     RetainsPairedResourcesUntilSubmissionCompletes) {
    int first_destructions{};
    int second_destructions{};

    Vulkan::VulkanQueueSubmissionState submissions{1U};
    Vulkan::VulkanResourceRetirementQueue queue;

    auto first = makeTrackedResource(first_destructions);
    auto second = makeTrackedResource(second_destructions);

    const auto submission_id = submissions.recordSubmission(0U);

    queue.retire(submission_id, first, second);

    queue.collectCompleted(submissions);

    EXPECT_EQ(first_destructions, 0);
    EXPECT_EQ(second_destructions, 0);

    submissions.recordCompletion(0U);
    queue.collectCompleted(submissions);

    EXPECT_EQ(first_destructions, 1);
    EXPECT_EQ(second_destructions, 1);
}

TEST(VulkanResourceRetirementQueueTests,
     DestroysPairedResourcesInReverseOrder) {
    std::array<int, 2> destruction_order{};
    std::size_t next_index{};

    Vulkan::VulkanQueueSubmissionState submissions{1U};
    Vulkan::VulkanResourceRetirementQueue queue;

    std::unique_ptr<int, OrderRecordingDeleter> first{
        new int{1},
        OrderRecordingDeleter{
            .destruction_order = &destruction_order,
            .next_index = &next_index,
            .resource_id = 1,
        },
    };

    std::unique_ptr<long, OrderRecordingDeleter> second{
        new long{2L},
        OrderRecordingDeleter{
            .destruction_order = &destruction_order,
            .next_index = &next_index,
            .resource_id = 2,
        },
    };

    const auto submission_id = submissions.recordSubmission(0U);

    queue.retire(submission_id, first, second);

    EXPECT_EQ(first.get(), nullptr);
    EXPECT_EQ(second.get(), nullptr);
    EXPECT_EQ(next_index, 0U);

    submissions.recordCompletion(0U);
    queue.collectCompleted(submissions);

    ASSERT_EQ(next_index, 2U);
    EXPECT_EQ(destruction_order[0], 2);
    EXPECT_EQ(destruction_order[1], 1);
}

TEST(VulkanResourceRetirementQueueTests,
     RetainsThreeResourcesUntilSubmissionCompletes) {
    int first_destructions{};
    int second_destructions{};
    int third_destructions{};

    Vulkan::VulkanQueueSubmissionState submissions{1U};
    Vulkan::VulkanResourceRetirementQueue queue;

    auto first = makeTrackedResource(first_destructions);
    auto second = makeTrackedResource(second_destructions);
    auto third = makeTrackedResource(third_destructions);

    const auto submission_id = submissions.recordSubmission(0U);

    queue.retire(submission_id, first, second, third);

    EXPECT_EQ(first.get(), nullptr);
    EXPECT_EQ(second.get(), nullptr);
    EXPECT_EQ(third.get(), nullptr);

    EXPECT_EQ(first_destructions, 0);
    EXPECT_EQ(second_destructions, 0);
    EXPECT_EQ(third_destructions, 0);

    queue.collectCompleted(submissions);

    EXPECT_EQ(first_destructions, 0);
    EXPECT_EQ(second_destructions, 0);
    EXPECT_EQ(third_destructions, 0);

    submissions.recordCompletion(0U);
    queue.collectCompleted(submissions);

    EXPECT_EQ(first_destructions, 1);
    EXPECT_EQ(second_destructions, 1);
    EXPECT_EQ(third_destructions, 1);

    queue.collectCompleted(submissions);

    EXPECT_EQ(first_destructions, 1);
    EXPECT_EQ(second_destructions, 1);
    EXPECT_EQ(third_destructions, 1);
}

TEST(VulkanResourceRetirementQueueTests, DestroysThreeResourcesInReverseOrder) {
    std::array<int, 3> destruction_order{};
    std::size_t next_index{};

    Vulkan::VulkanQueueSubmissionState submissions{1U};
    Vulkan::VulkanResourceRetirementQueue queue;

    std::unique_ptr<int, TripleOrderRecordingDeleter> first{
        new int{1},
        TripleOrderRecordingDeleter{
            .destruction_order = &destruction_order,
            .next_index = &next_index,
            .resource_id = 1,
        },
    };

    std::unique_ptr<long, TripleOrderRecordingDeleter> second{
        new long{2L},
        TripleOrderRecordingDeleter{
            .destruction_order = &destruction_order,
            .next_index = &next_index,
            .resource_id = 2,
        },
    };

    std::unique_ptr<double, TripleOrderRecordingDeleter> third{
        new double{},
        TripleOrderRecordingDeleter{
            .destruction_order = &destruction_order,
            .next_index = &next_index,
            .resource_id = 3,
        },
    };

    const auto submission_id = submissions.recordSubmission(0U);

    queue.retire(submission_id, first, second, third);

    EXPECT_EQ(first.get(), nullptr);
    EXPECT_EQ(second.get(), nullptr);
    EXPECT_EQ(third.get(), nullptr);
    EXPECT_EQ(next_index, 0U);

    submissions.recordCompletion(0U);
    queue.collectCompleted(submissions);

    ASSERT_EQ(next_index, 3U);
    EXPECT_EQ(destruction_order[0], 3);
    EXPECT_EQ(destruction_order[1], 2);
    EXPECT_EQ(destruction_order[2], 1);
}

TEST(VulkanResourceRetirementQueueTests,
     RejectsRetiringThreeResourcesWithDuplicateFirstAndSecond) {
    ASSERT_DEATH(([] -> void {
                     Vulkan::VulkanResourceRetirementQueue queue;

                     int first_destructions{};
                     int third_destructions{};

                     auto first = makeTrackedResource(first_destructions);
                     auto &same_as_first = first;

                     std::unique_ptr<long, CountingDeleter> third{
                         new long{},
                         CountingDeleter{&third_destructions},
                     };

                     queue.retire(0U, first, same_as_first, third);
                 }()),
                 "requires distinct resource objects");
}

TEST(VulkanResourceRetirementQueueTests,
     RejectsRetiringThreeResourcesWithDuplicateFirstAndThird) {
    ASSERT_DEATH(
        {
            Vulkan::VulkanResourceRetirementQueue queue;

            int first_destructions{};
            int second_destructions{};

            auto first = makeTrackedResource(first_destructions);
            auto second = makeTrackedResource(second_destructions);

            queue.retire(0U, first, second, first);
        },
        "requires distinct resource objects");
}

TEST(VulkanResourceRetirementQueueTests,
     RejectsRetiringThreeResourcesWithDuplicateSecondAndThird) {
    ASSERT_DEATH(
        {
            Vulkan::VulkanResourceRetirementQueue queue;

            int first_destructions{};
            int second_destructions{};

            auto first = makeTrackedResource(first_destructions);
            auto second = makeTrackedResource(second_destructions);

            queue.retire(0U, first, second, second);
        },
        "requires distinct resource objects");
}

TEST(VulkanResourceRetirementQueueTests, RejectsRetiringTheSameResourceTwice) {
    ASSERT_DEATH(
        {
            int destruction_count{};

            Vulkan::VulkanResourceRetirementQueue queue;
            auto resource = makeTrackedResource(destruction_count);

            queue.retire(0U, resource, resource);
        },
        "requires distinct resource objects");
}
