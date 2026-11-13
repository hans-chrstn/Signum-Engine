#include "engine/core/time/backlog_policy.hpp"
#include "engine/core/time/backlog_policy_controller.hpp"
#include "engine/core/time/controlled_fixed_update_cycle.hpp"
#include <cstddef>
#include <gtest/gtest.h>
#include <stdexcept>

namespace Time = SNE::Engine::Core::Time;

namespace {
    constexpr Time::Duration kFixedStep{0.125};
    constexpr Time::Duration kHalfStep{0.0625};
    constexpr Time::Duration kOneAndHalfSteps{0.1875};
    constexpr Time::Duration kTwoAndHalfSteps{0.3125};
    constexpr Time::Duration kThreeAndHalfSteps{0.4375};
    constexpr Time::Duration kFourAndHalfSteps{0.5625};
    constexpr Time::Duration kTwoSteps{0.25};

    constexpr std::size_t kOneUpdate{1};
    constexpr std::size_t kTwoUpdates{2};
} // namespace

TEST(ControlledFixedUpdateCycleTests, UsesInitialActivePolicy) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    Time::BacklogPolicyController controller{Time::preserveBacklog};

    scheduler.accumulate(kThreeAndHalfSteps);

    std::size_t callback_count{};

    const Time::FixedUpdateCycleResult result =
        Time::runControlledFixedUpdateCycle(
            scheduler, kTwoUpdates,
            [&](Time::Duration step) -> void {
                EXPECT_EQ(step, kFixedStep);
                ++callback_count;
            },
            controller);

    EXPECT_EQ(result.updates_executed, kTwoUpdates);
    EXPECT_EQ(result.time_discarded, Time::Duration::zero());
    EXPECT_EQ(result.remaining_time, kOneAndHalfSteps);
    EXPECT_EQ(scheduler.accumulatedTime(), kOneAndHalfSteps);
    EXPECT_EQ(callback_count, kTwoUpdates);
}

TEST(ControlledFixedUpdateCycleTests,
     ActivatesPendingPolicyBeforeExecutingCycle) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    Time::BacklogPolicyController controller{Time::preserveBacklog};

    scheduler.accumulate(kFourAndHalfSteps);
    controller.requestPolicy(Time::discardExcessBacklog);

    const Time::FixedUpdateCycleResult result =
        Time::runControlledFixedUpdateCycle(
            scheduler, kTwoUpdates, [](Time::Duration) -> void {}, controller);

    EXPECT_EQ(result.updates_executed, kTwoUpdates);
    EXPECT_EQ(result.time_discarded, kTwoSteps);
    EXPECT_EQ(result.remaining_time, kHalfStep);
    EXPECT_EQ(scheduler.accumulatedTime(), kHalfStep);
    EXPECT_FALSE(controller.commitPendingPolicy());
}

TEST(ControlledFixedUpdateCycleTests,
     PolicyRequestedDuringUpdatesActivatesNextCycle) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    Time::BacklogPolicyController controller{Time::preserveBacklog};

    scheduler.accumulate(kFourAndHalfSteps);

    std::size_t callback_count{};

    const Time::FixedUpdateCycleResult first =
        Time::runControlledFixedUpdateCycle(
            scheduler, kTwoUpdates,
            [&](Time::Duration) -> void {
                ++callback_count;

                if (callback_count == kOneUpdate) {
                    controller.requestPolicy(Time::discardExcessBacklog);
                }
            },
            controller);

    EXPECT_EQ(first.updates_executed, kTwoUpdates);
    EXPECT_EQ(first.time_discarded, Time::Duration::zero());
    EXPECT_EQ(first.remaining_time, kTwoAndHalfSteps);

    const Time::FixedUpdateCycleResult second =
        Time::runControlledFixedUpdateCycle(
            scheduler, kOneUpdate,
            [&](Time::Duration) -> void { ++callback_count; }, controller);

    EXPECT_EQ(second.updates_executed, kOneUpdate);
    EXPECT_EQ(second.time_discarded, kFixedStep);
    EXPECT_EQ(second.remaining_time, kHalfStep);
    EXPECT_EQ(callback_count, 3U);
    EXPECT_EQ(scheduler.accumulatedTime(), kHalfStep);
}

TEST(ControlledFixedUpdateCycleTests,
     PolicySnapshotSurvivesCommitDuringUpdateCallback) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    Time::BacklogPolicyController controller{Time::preserveBacklog};

    scheduler.accumulate(kFourAndHalfSteps);

    std::size_t callback_count{};

    const Time::FixedUpdateCycleResult result =
        Time::runControlledFixedUpdateCycle(
            scheduler, kTwoUpdates,
            [&](Time::Duration) -> void {
                ++callback_count;

                if (callback_count == kOneUpdate) {
                    controller.requestPolicy(Time::discardExcessBacklog);
                    EXPECT_TRUE(controller.commitPendingPolicy());
                }
            },
            controller);

    EXPECT_EQ(result.updates_executed, kTwoUpdates);
    EXPECT_EQ(result.time_discarded, Time::Duration::zero());
    EXPECT_EQ(result.remaining_time, kTwoAndHalfSteps);

    EXPECT_EQ(callback_count, kTwoUpdates);
    EXPECT_EQ(scheduler.accumulatedTime(), kTwoAndHalfSteps);
}

TEST(ControlledFixedUpdateCycleTests,
     FailedUpdateLeavesRequestedPolicyPending) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    Time::BacklogPolicyController controller{Time::preserveBacklog};

    scheduler.accumulate(kThreeAndHalfSteps);

    EXPECT_THROW(static_cast<void>(Time::runControlledFixedUpdateCycle(
                     scheduler, kTwoUpdates,
                     [&](Time::Duration) -> void {
                         controller.requestPolicy(Time::discardExcessBacklog);
                         throw std::runtime_error{"Simulation update failed"};
                     },
                     controller)),
                 std::runtime_error);

    EXPECT_EQ(scheduler.accumulatedTime(), kTwoAndHalfSteps);

    const Time::FixedUpdateCycleResult next =
        Time::runControlledFixedUpdateCycle(
            scheduler, kOneUpdate, [](Time::Duration) -> void {}, controller);

    EXPECT_EQ(next.updates_executed, kOneUpdate);
    EXPECT_EQ(next.time_discarded, kFixedStep);
    EXPECT_EQ(next.remaining_time, kHalfStep);
}
