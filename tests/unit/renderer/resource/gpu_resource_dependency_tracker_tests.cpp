#include "engine/renderer/resource/gpu_resource_dependency_tracker.hpp"
#include <cstdint>
#include <gtest/gtest.h>

namespace Renderer = SNE::Engine::Renderer;

namespace {
    constexpr std::uint64_t kDependentRegistryId{2U};
    constexpr std::uint64_t kFirstDependencyRegistryId{3U};
    constexpr std::uint64_t kSecondDependencyRegistryId{4U};

    constexpr std::uint32_t kReplacementGeneration{2U};

    constexpr Renderer::GpuResourceHandle kDependentHandle{
        .index = 0U,
        .generation = 1U,
        .registry_id = kDependentRegistryId,
    };

    constexpr Renderer::GpuResourceHandle kOtherDependentHandle{
        .index = 1U,
        .generation = 1U,
        .registry_id = kDependentRegistryId,
    };

    constexpr Renderer::GpuResourceHandle kFirstDependencyHandle{
        .index = 0U,
        .generation = 1U,
        .registry_id = kFirstDependencyRegistryId,
    };

    constexpr Renderer::GpuResourceHandle kSecondDependencyHandle{
        .index = 0U,
        .generation = 1U,
        .registry_id = kSecondDependencyRegistryId,
    };

    constexpr Renderer::GpuResourceHandle kReusedDependencyHandle{
        .index = 0U,
        .generation = kReplacementGeneration,
        .registry_id = kFirstDependencyRegistryId,
    };

    constexpr Renderer::GpuResourceDependency kRelationship{
        .dependent = kDependentHandle,
        .dependency = kFirstDependencyHandle,
    };
} // namespace

TEST(GpuResourceDependencyTrackerTests, StartsEmpty) {
    const Renderer::GpuResourceDependencyTracker tracker;

    EXPECT_FALSE(tracker.contains(kRelationship));
    EXPECT_FALSE(tracker.contains({}));
}

TEST(GpuResourceDependencyTrackerTests, RegistersDependency) {
    Renderer::GpuResourceDependencyTracker tracker;

    tracker.addDependency(kRelationship);

    EXPECT_TRUE(tracker.contains(kRelationship));
}

TEST(GpuResourceDependencyTrackerTests, RelationshipDirectionMatters) {
    Renderer::GpuResourceDependencyTracker tracker;

    tracker.addDependency(kRelationship);

    const Renderer::GpuResourceDependency reversed{
        .dependent = kFirstDependencyHandle,
        .dependency = kDependentHandle,
    };

    EXPECT_TRUE(tracker.contains(kRelationship));
    EXPECT_FALSE(tracker.contains(reversed));
}

TEST(GpuResourceDependencyTrackerTests,
     AllowsMultipleDependenciesForOneResource) {
    Renderer::GpuResourceDependencyTracker tracker;

    const Renderer::GpuResourceDependency first{
        .dependent = kDependentHandle,
        .dependency = kFirstDependencyHandle,
    };

    const Renderer::GpuResourceDependency second{
        .dependent = kDependentHandle,
        .dependency = kSecondDependencyHandle,
    };

    tracker.addDependency(first);
    tracker.addDependency(second);

    EXPECT_TRUE(tracker.contains(first));
    EXPECT_TRUE(tracker.contains(second));
}

TEST(GpuResourceDependencyTrackerTests,
     AllowsMultipleResourcesToShareDependency) {
    Renderer::GpuResourceDependencyTracker tracker;

    const Renderer::GpuResourceDependency first{
        .dependent = kDependentHandle,
        .dependency = kFirstDependencyHandle,
    };

    const Renderer::GpuResourceDependency second{
        .dependent = kOtherDependentHandle,
        .dependency = kFirstDependencyHandle,
    };

    tracker.addDependency(first);
    tracker.addDependency(second);

    EXPECT_TRUE(tracker.contains(first));
    EXPECT_TRUE(tracker.contains(second));
}

TEST(GpuResourceDependencyTrackerTests, DistinguishesResourceGenerations) {
    Renderer::GpuResourceDependencyTracker tracker;

    const Renderer::GpuResourceDependency other_generation{
        .dependent = kDependentHandle,
        .dependency = kReusedDependencyHandle,
    };

    tracker.addDependency(kRelationship);

    EXPECT_TRUE(tracker.contains(kRelationship));
    EXPECT_FALSE(tracker.contains(other_generation));
}

TEST(GpuResourceDependencyTrackerTests, RejectsInvalidDependentHandle) {
    Renderer::GpuResourceDependencyTracker tracker;

    const Renderer::GpuResourceDependency missing_registry{
        .dependent =
            {
                .index = 0U,
                .generation = 1U,
                .registry_id = 0U,
            },
        .dependency = kFirstDependencyHandle,
    };

    const Renderer::GpuResourceDependency missing_generation{
        .dependent =
            {
                .index = 0U,
                .generation = 0U,
                .registry_id = kDependentRegistryId,
            },
        .dependency = kFirstDependencyHandle,
    };

    ASSERT_DEATH(
        { tracker.addDependency(missing_registry); },
        "requires a valid dependent handle");

    ASSERT_DEATH(
        { tracker.addDependency(missing_generation); },
        "requires a valid dependent handle");
}

TEST(GpuResourceDependencyTrackerTests, RejectsInvalidDependencyHandle) {
    Renderer::GpuResourceDependencyTracker tracker;

    const Renderer::GpuResourceDependency missing_registry{
        .dependent = kDependentHandle,
        .dependency =
            {
                .index = 0U,
                .generation = 1U,
                .registry_id = 0U,
            },
    };

    const Renderer::GpuResourceDependency missing_generation{
        .dependent = kDependentHandle,
        .dependency =
            {
                .index = 0U,
                .generation = 0U,
                .registry_id = kFirstDependencyRegistryId,
            },
    };

    ASSERT_DEATH(
        { tracker.addDependency(missing_registry); },
        "requires a valid dependency handle");

    ASSERT_DEATH(
        { tracker.addDependency(missing_generation); },
        "requires a valid dependency handle");
}

TEST(GpuResourceDependencyTrackerTests, RejectsSelfDependency) {
    Renderer::GpuResourceDependencyTracker tracker;

    const Renderer::GpuResourceDependency self_dependency{
        .dependent = kDependentHandle,
        .dependency = kDependentHandle,
    };

    ASSERT_DEATH(
        { tracker.addDependency(self_dependency); },
        "cannot reference the same resource");
}

TEST(GpuResourceDependencyTrackerTests, RejectsDuplicateRelationship) {
    Renderer::GpuResourceDependencyTracker tracker;

    tracker.addDependency(kRelationship);

    ASSERT_DEATH(
        { tracker.addDependency(kRelationship); },
        "requires a unique relationship");

    EXPECT_TRUE(tracker.contains(kRelationship));
}

TEST(GpuResourceDependencyTrackerTests, RemovesExistingRelationship) {
    Renderer::GpuResourceDependencyTracker tracker;

    tracker.addDependency(kRelationship);

    EXPECT_TRUE(tracker.contains(kRelationship));
    EXPECT_TRUE(tracker.removeDependency(kRelationship));
    EXPECT_FALSE(tracker.contains(kRelationship));
}

TEST(GpuResourceDependencyTrackerTests, RejectsRemovingMissingRelationship) {
    Renderer::GpuResourceDependencyTracker tracker;

    EXPECT_FALSE(tracker.removeDependency(kRelationship));

    tracker.addDependency(kRelationship);
    EXPECT_TRUE(tracker.removeDependency(kRelationship));
    EXPECT_FALSE(tracker.removeDependency(kRelationship));
}

TEST(GpuResourceDependencyTrackerTests, RemovalPreservesOtherRelationships) {
    Renderer::GpuResourceDependencyTracker tracker;

    const Renderer::GpuResourceDependency other{
        .dependent = kDependentHandle,
        .dependency = kSecondDependencyHandle,
    };

    tracker.addDependency(kRelationship);
    tracker.addDependency(other);

    EXPECT_TRUE(tracker.removeDependency(kRelationship));

    EXPECT_FALSE(tracker.contains(kRelationship));
    EXPECT_TRUE(tracker.contains(other));
}

TEST(GpuResourceDependencyTrackerTests, RemovalRespectsRelationshipDirection) {
    Renderer::GpuResourceDependencyTracker tracker;

    const Renderer::GpuResourceDependency reversed{
        .dependent = kFirstDependencyHandle,
        .dependency = kDependentHandle,
    };

    tracker.addDependency(kRelationship);

    EXPECT_FALSE(tracker.removeDependency(reversed));
    EXPECT_TRUE(tracker.contains(kRelationship));
}

TEST(GpuResourceDependencyTrackerTests, RemovalRespectsResourceGeneration) {
    Renderer::GpuResourceDependencyTracker tracker;

    const Renderer::GpuResourceDependency other_generation{
        .dependent = kDependentHandle,
        .dependency = kReusedDependencyHandle,
    };

    tracker.addDependency(kRelationship);

    EXPECT_FALSE(tracker.removeDependency(other_generation));
    EXPECT_TRUE(tracker.contains(kRelationship));
}

TEST(GpuResourceDependencyTrackerTests,
     RemovesAllRelationshipsWithMatchingDependency) {
    Renderer::GpuResourceDependencyTracker tracker;

    constexpr std::size_t kExpectedRemovedCount{2U};

    const Renderer::GpuResourceDependency second{
        .dependent = kOtherDependentHandle,
        .dependency = kFirstDependencyHandle,
    };

    const Renderer::GpuResourceDependency unrelated{
        .dependent = kDependentHandle,
        .dependency = kSecondDependencyHandle,
    };

    tracker.addDependency(kRelationship);
    tracker.addDependency(second);
    tracker.addDependency(unrelated);

    EXPECT_EQ(tracker.removeRelationshipsInvolving(kFirstDependencyHandle),
              kExpectedRemovedCount);

    EXPECT_FALSE(tracker.contains(kRelationship));
    EXPECT_FALSE(tracker.contains(second));
    EXPECT_TRUE(tracker.contains(unrelated));
}

TEST(GpuResourceDependencyTrackerTests,
     RemovesAllRelationshipsWithMatchingDependent) {
    Renderer::GpuResourceDependencyTracker tracker;

    constexpr std::size_t kExpectedRemovedCount{2U};

    const Renderer::GpuResourceDependency second{
        .dependent = kDependentHandle,
        .dependency = kSecondDependencyHandle,
    };

    const Renderer::GpuResourceDependency unrelated{
        .dependent = kOtherDependentHandle,
        .dependency = kFirstDependencyHandle,
    };

    tracker.addDependency(kRelationship);
    tracker.addDependency(second);
    tracker.addDependency(unrelated);

    EXPECT_EQ(tracker.removeRelationshipsInvolving(kDependentHandle),
              kExpectedRemovedCount);

    EXPECT_FALSE(tracker.contains(kRelationship));
    EXPECT_FALSE(tracker.contains(second));
    EXPECT_TRUE(tracker.contains(unrelated));
}

TEST(GpuResourceDependencyTrackerTests,
     RemovesRelationshipsFromBothDirections) {
    Renderer::GpuResourceDependencyTracker tracker;

    constexpr std::size_t kExpectedRemovedCount{2U};

    const Renderer::GpuResourceDependency outgoing{
        .dependent = kFirstDependencyHandle,
        .dependency = kSecondDependencyHandle,
    };

    const Renderer::GpuResourceDependency unrelated{
        .dependent = kOtherDependentHandle,
        .dependency = kSecondDependencyHandle,
    };

    tracker.addDependency(kRelationship);
    tracker.addDependency(outgoing);
    tracker.addDependency(unrelated);

    EXPECT_EQ(tracker.removeRelationshipsInvolving(kFirstDependencyHandle),
              kExpectedRemovedCount);

    EXPECT_FALSE(tracker.contains(kRelationship));
    EXPECT_FALSE(tracker.contains(outgoing));
    EXPECT_TRUE(tracker.contains(unrelated));
}

TEST(GpuResourceDependencyTrackerTests,
     RemovingUnknownResourcePreservesRelationships) {
    Renderer::GpuResourceDependencyTracker tracker;

    tracker.addDependency(kRelationship);

    EXPECT_EQ(tracker.removeRelationshipsInvolving({}), 0U);

    EXPECT_EQ(tracker.removeRelationshipsInvolving(kReusedDependencyHandle),
              0U);

    EXPECT_TRUE(tracker.contains(kRelationship));
}

TEST(GpuResourceDependencyTrackerTests, RepeatedResourceRemovalReturnsZero) {
    Renderer::GpuResourceDependencyTracker tracker;

    tracker.addDependency(kRelationship);

    EXPECT_EQ(tracker.removeRelationshipsInvolving(kFirstDependencyHandle), 1U);

    EXPECT_EQ(tracker.removeRelationshipsInvolving(kFirstDependencyHandle), 0U);

    EXPECT_FALSE(tracker.contains(kRelationship));
}

TEST(GpuResourceDependencyTrackerTests,
     ResourceRemovalRespectsRegistryIdentity) {
    Renderer::GpuResourceDependencyTracker tracker;

    const Renderer::GpuResourceDependency other_registry{
        .dependent = kDependentHandle,
        .dependency = kSecondDependencyHandle,
    };

    tracker.addDependency(kRelationship);
    tracker.addDependency(other_registry);

    EXPECT_EQ(tracker.removeRelationshipsInvolving(kFirstDependencyHandle), 1U);

    EXPECT_FALSE(tracker.contains(kRelationship));
    EXPECT_TRUE(tracker.contains(other_registry));
}

TEST(GpuResourceDependencyTrackerTests, EmptyTrackerHasNoDependents) {
    const Renderer::GpuResourceDependencyTracker tracker;

    EXPECT_FALSE(tracker.hasDependents(kFirstDependencyHandle));
    EXPECT_FALSE(tracker.hasDependents(kDependentHandle));
    EXPECT_FALSE(tracker.hasDependents({}));
}

TEST(GpuResourceDependencyTrackerTests, FindsRegisteredDependents) {
    Renderer::GpuResourceDependencyTracker tracker;

    tracker.addDependency(kRelationship);

    const Renderer::GpuResourceDependencyTracker &const_tracker = tracker;

    EXPECT_TRUE(const_tracker.hasDependents(kFirstDependencyHandle));
    EXPECT_FALSE(const_tracker.hasDependents(kDependentHandle));
}

TEST(GpuResourceDependencyTrackerTests, FindsMultipleDependentsOfSameResource) {
    Renderer::GpuResourceDependencyTracker tracker;

    const Renderer::GpuResourceDependency second{
        .dependent = kOtherDependentHandle,
        .dependency = kFirstDependencyHandle,
    };

    tracker.addDependency(kRelationship);
    tracker.addDependency(second);

    EXPECT_TRUE(tracker.hasDependents(kFirstDependencyHandle));

    EXPECT_TRUE(tracker.removeDependency(kRelationship));
    EXPECT_TRUE(tracker.hasDependents(kFirstDependencyHandle));

    EXPECT_TRUE(tracker.removeDependency(second));
    EXPECT_FALSE(tracker.hasDependents(kFirstDependencyHandle));
}

TEST(GpuResourceDependencyTrackerTests,
     DistinguishesDependentsFromDependencies) {
    Renderer::GpuResourceDependencyTracker tracker;

    const Renderer::GpuResourceDependency second{
        .dependent = kFirstDependencyHandle,
        .dependency = kSecondDependencyHandle,
    };

    tracker.addDependency(kRelationship);
    tracker.addDependency(second);

    EXPECT_FALSE(tracker.hasDependents(kDependentHandle));
    EXPECT_TRUE(tracker.hasDependents(kFirstDependencyHandle));
    EXPECT_TRUE(tracker.hasDependents(kSecondDependencyHandle));
}

TEST(GpuResourceDependencyTrackerTests,
     DependentLookupRespectsResourceIdentity) {
    Renderer::GpuResourceDependencyTracker tracker;

    tracker.addDependency(kRelationship);

    EXPECT_TRUE(tracker.hasDependents(kFirstDependencyHandle));

    EXPECT_FALSE(tracker.hasDependents(kReusedDependencyHandle));
    EXPECT_FALSE(tracker.hasDependents(kSecondDependencyHandle));
}

TEST(GpuResourceDependencyTrackerTests,
     RemovingResourceRelationshipsClearsDependents) {
    Renderer::GpuResourceDependencyTracker tracker;

    tracker.addDependency(kRelationship);

    EXPECT_TRUE(tracker.hasDependents(kFirstDependencyHandle));

    EXPECT_EQ(tracker.removeRelationshipsInvolving(kFirstDependencyHandle), 1U);

    EXPECT_FALSE(tracker.hasDependents(kFirstDependencyHandle));
}

TEST(GpuResourceDependencyTrackerTests, RejectsTwoResourceCycle) {
    Renderer::GpuResourceDependencyTracker tracker;

    tracker.addDependency(kRelationship);

    const Renderer::GpuResourceDependency reverse_relationship{
        .dependent = kFirstDependencyHandle,
        .dependency = kDependentHandle,
    };

    EXPECT_DEATH(tracker.addDependency(reverse_relationship),
                 "dependency cycle");
}

TEST(GpuResourceDependencyTrackerTests, RejectsIndirectDependencyCycle) {
    Renderer::GpuResourceDependencyTracker tracker;

    const Renderer::GpuResourceDependency second_relationship{
        .dependent = kFirstDependencyHandle,
        .dependency = kSecondDependencyHandle,
    };

    const Renderer::GpuResourceDependency cycle_relationship{
        .dependent = kSecondDependencyHandle,
        .dependency = kDependentHandle,
    };

    tracker.addDependency(kRelationship);
    tracker.addDependency(second_relationship);

    EXPECT_DEATH(tracker.addDependency(cycle_relationship), "dependency cycle");
}

TEST(GpuResourceDependencyTrackerTests, RejectsCycleThroughBranchingGraph) {
    Renderer::GpuResourceDependencyTracker tracker;

    const Renderer::GpuResourceDependency second_branch{
        .dependent = kDependentHandle,
        .dependency = kSecondDependencyHandle,
    };

    const Renderer::GpuResourceDependency first_branch_end{
        .dependent = kFirstDependencyHandle,
        .dependency = kOtherDependentHandle,
    };

    const Renderer::GpuResourceDependency second_branch_end{
        .dependent = kSecondDependencyHandle,
        .dependency = kOtherDependentHandle,
    };

    const Renderer::GpuResourceDependency cycle_relationship{
        .dependent = kOtherDependentHandle,
        .dependency = kDependentHandle,
    };

    tracker.addDependency(kRelationship);
    tracker.addDependency(second_branch);
    tracker.addDependency(first_branch_end);
    tracker.addDependency(second_branch_end);

    EXPECT_DEATH(tracker.addDependency(cycle_relationship), "dependency cycle");
}

TEST(GpuResourceDependencyTrackerTests, AllowsAcyclicBranchingGraph) {
    Renderer::GpuResourceDependencyTracker tracker;

    const Renderer::GpuResourceDependency second_branch{
        .dependent = kDependentHandle,
        .dependency = kSecondDependencyHandle,
    };

    const Renderer::GpuResourceDependency first_branch_end{
        .dependent = kFirstDependencyHandle,
        .dependency = kOtherDependentHandle,
    };

    const Renderer::GpuResourceDependency second_branch_end{
        .dependent = kSecondDependencyHandle,
        .dependency = kOtherDependentHandle,
    };

    tracker.addDependency(kRelationship);
    tracker.addDependency(second_branch);
    tracker.addDependency(first_branch_end);
    tracker.addDependency(second_branch_end);

    EXPECT_TRUE(tracker.contains(kRelationship));
    EXPECT_TRUE(tracker.contains(second_branch));
    EXPECT_TRUE(tracker.contains(first_branch_end));
    EXPECT_TRUE(tracker.contains(second_branch_end));
}

TEST(GpuResourceDependencyTrackerTests, AllowsRelationshipAfterPathRemoval) {
    Renderer::GpuResourceDependencyTracker tracker;

    const Renderer::GpuResourceDependency second_relationship{
        .dependent = kFirstDependencyHandle,
        .dependency = kSecondDependencyHandle,
    };

    const Renderer::GpuResourceDependency new_relationship{
        .dependent = kSecondDependencyHandle,
        .dependency = kDependentHandle,
    };

    tracker.addDependency(kRelationship);
    tracker.addDependency(second_relationship);

    EXPECT_TRUE(tracker.removeDependency(second_relationship));

    tracker.addDependency(new_relationship);

    EXPECT_TRUE(tracker.contains(kRelationship));
    EXPECT_FALSE(tracker.contains(second_relationship));
    EXPECT_TRUE(tracker.contains(new_relationship));
}

TEST(GpuResourceDependencyTrackerTests, CycleDetectionRespectsGenerations) {
    Renderer::GpuResourceDependencyTracker tracker;

    const Renderer::GpuResourceDependency new_generation_relationship{
        .dependent = kReusedDependencyHandle,
        .dependency = kSecondDependencyHandle,
    };

    const Renderer::GpuResourceDependency new_relationship{
        .dependent = kSecondDependencyHandle,
        .dependency = kDependentHandle,
    };

    tracker.addDependency(kRelationship);
    tracker.addDependency(new_generation_relationship);
    tracker.addDependency(new_relationship);

    EXPECT_TRUE(tracker.contains(kRelationship));
    EXPECT_TRUE(tracker.contains(new_generation_relationship));
    EXPECT_TRUE(tracker.contains(new_relationship));
}
