#include "engine/core/time/backlog_policy.hpp"
#include "engine/core/time/backlog_policy_controller.hpp"
#include "engine/core/time/controlled_fixed_update_cycle.hpp"
#include <array>
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

TEST(ControlledFixedUpdateCycleTests,
     RepeatedCyclesWithoutNewRequestRetainActivePolicy) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    Time::BacklogPolicyController controller{Time::preserveBacklog};

    scheduler.accumulate(kFourAndHalfSteps);
    controller.requestPolicy(Time::discardExcessBacklog);

    const auto first = Time::runControlledFixedUpdateCycle(
        scheduler, kTwoUpdates, [](Time::Duration) -> void {}, controller);

    EXPECT_EQ(first.time_discarded, kTwoSteps);
    EXPECT_EQ(first.remaining_time, kHalfStep);

    scheduler.accumulate(kTwoSteps);

    const auto second = Time::runControlledFixedUpdateCycle(
        scheduler, kOneUpdate, [](Time::Duration) -> void {}, controller);

    EXPECT_EQ(second.updates_executed, kOneUpdate);
    EXPECT_EQ(second.time_discarded, kFixedStep);
    EXPECT_EQ(second.remaining_time, kHalfStep);
}

TEST(ControlledFixedUpdateCycleTests,
     LatestPendingRequestWinsAtSchedulingBoundary) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    Time::BacklogPolicyController controller{Time::preserveBacklog};

    scheduler.accumulate(kFourAndHalfSteps);

    controller.requestPolicy(Time::discardExcessBacklog);
    controller.requestPolicy(Time::preserveBacklog);

    const auto result = Time::runControlledFixedUpdateCycle(
        scheduler, kTwoUpdates, [](Time::Duration) -> void {}, controller);

    EXPECT_EQ(result.updates_executed, kTwoUpdates);
    EXPECT_EQ(result.time_discarded, Time::Duration::zero());
    EXPECT_EQ(result.remaining_time, kTwoAndHalfSteps);
}

TEST(ControlledFixedUpdateCycleTests,
     SupportsSuccessiveTransitionsWithoutResettingScheduler) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    Time::BacklogPolicyController controller{Time::preserveBacklog};

    scheduler.accumulate(kFourAndHalfSteps);
    controller.requestPolicy(Time::discardExcessBacklog);

    const auto first = Time::runControlledFixedUpdateCycle(
        scheduler, kTwoUpdates, [](Time::Duration) -> void {}, controller);

    EXPECT_EQ(first.time_discarded, kTwoSteps);
    EXPECT_EQ(first.remaining_time, kHalfStep);

    controller.requestPolicy(Time::preserveBacklog);
    scheduler.accumulate(kTwoSteps);

    const auto second = Time::runControlledFixedUpdateCycle(
        scheduler, kOneUpdate, [](Time::Duration) -> void {}, controller);

    EXPECT_EQ(second.updates_executed, kOneUpdate);
    EXPECT_EQ(second.time_discarded, Time::Duration::zero());
    EXPECT_EQ(second.remaining_time, kOneAndHalfSteps);
    EXPECT_EQ(scheduler.fixedStep(), kFixedStep);
}

TEST(ControlledFixedUpdateCycleTests,
     RejectsNestedCycleAndPreservesPendingRequest) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    Time::BacklogPolicyController controller{Time::preserveBacklog};

    scheduler.accumulate(kTwoAndHalfSteps);

    const auto first = Time::runControlledFixedUpdateCycle(
        scheduler, kOneUpdate,
        [&](Time::Duration) -> void {
            controller.requestPolicy(Time::discardExcessBacklog);

            EXPECT_DEATH(
                static_cast<void>(Time::runControlledFixedUpdateCycle(
                    scheduler, 0U, [](Time::Duration) -> void {}, controller)),
                "Controlled fixed-update cycle requires no cycle to be in "
                "progress");
        },
        controller);

    EXPECT_EQ(first.updates_executed, kOneUpdate);
    EXPECT_EQ(first.time_discarded, Time::Duration::zero());
    EXPECT_EQ(first.remaining_time, kOneAndHalfSteps);

    const auto second = Time::runControlledFixedUpdateCycle(
        scheduler, 0U, [](Time::Duration) -> void {}, controller);

    EXPECT_EQ(second.updates_executed, 0U);
    EXPECT_EQ(second.time_discarded, kFixedStep);
    EXPECT_EQ(second.remaining_time, kHalfStep);
}

TEST(ControlledFixedUpdateCycleTests, PolicyExceptionReleasesCycleGuard) {
    Time::FixedStepScheduler scheduler{kFixedStep};

    Time::BacklogPolicyController controller{
        [](const Time::BacklogContext &) -> Time::BacklogDecision {
            throw std::runtime_error{"Backlog policy failed"};
        }};

    scheduler.accumulate(kOneAndHalfSteps);

    EXPECT_THROW(
        static_cast<void>(Time::runControlledFixedUpdateCycle(
            scheduler, kOneUpdate, [](Time::Duration) -> void {}, controller)),
        std::runtime_error);

    EXPECT_EQ(scheduler.accumulatedTime(), kHalfStep);

    controller.requestPolicy(Time::preserveBacklog);

    const auto result = Time::runControlledFixedUpdateCycle(
        scheduler, 0U, [](Time::Duration) -> void {}, controller);

    EXPECT_EQ(result.updates_executed, 0U);
    EXPECT_EQ(result.time_discarded, Time::Duration::zero());
    EXPECT_EQ(result.remaining_time, kHalfStep);
}

TEST(ControlledFixedUpdateCycleTests,
     PolicyTransitionPreservesSimulationStateAndAccountsForDiscardedTime) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    Time::BacklogPolicyController controller{Time::preserveBacklog};

    std::size_t position{};
    std::size_t simulated_ticks{};
    Time::Duration simulation_time{};

    const auto update = [&](Time::Duration step) -> void {
        EXPECT_EQ(step, kFixedStep);
        ++position;
        ++simulated_ticks;
        simulation_time += step;
    };

    scheduler.accumulate(kFourAndHalfSteps);
    controller.requestPolicy(Time::discardExcessBacklog);

    const auto first = Time::runControlledFixedUpdateCycle(
        scheduler, kTwoUpdates, update, controller);

    EXPECT_EQ(first.updates_executed, kTwoUpdates);
    EXPECT_EQ(first.time_discarded, kTwoSteps);
    EXPECT_EQ(first.remaining_time, kHalfStep);
    EXPECT_EQ(position, 2U);
    EXPECT_EQ(simulated_ticks, 2U);
    EXPECT_EQ(simulation_time, kTwoSteps);

    controller.requestPolicy(Time::preserveBacklog);

    const auto transition =
        Time::runControlledFixedUpdateCycle(scheduler, 0U, update, controller);

    EXPECT_EQ(transition.updates_executed, 0U);
    EXPECT_EQ(transition.time_discarded, Time::Duration::zero());
    EXPECT_EQ(transition.remaining_time, kHalfStep);
    EXPECT_EQ(position, 2U);
    EXPECT_EQ(simulated_ticks, 2U);

    scheduler.accumulate(kHalfStep);

    const auto next = Time::runControlledFixedUpdateCycle(scheduler, kOneUpdate,
                                                          update, controller);

    EXPECT_EQ(next.updates_executed, kOneUpdate);
    EXPECT_EQ(next.time_discarded, Time::Duration::zero());
    EXPECT_EQ(next.remaining_time, Time::Duration::zero());

    EXPECT_EQ(position, 3U);
    EXPECT_EQ(simulated_ticks, 3U);
    EXPECT_EQ(simulation_time, Time::Duration{0.375});

    EXPECT_EQ(simulation_time + first.time_discarded +
                  scheduler.accumulatedTime(),
              kFourAndHalfSteps + kHalfStep);
}

TEST(ControlledFixedUpdateCycleTests,
     IdenticalPolicyTransitionsProduceDeterministicReplay) {
    struct ReplayTrace {
        std::array<Time::FixedUpdateCycleResult, 4> results{};
        std::array<std::size_t, 4> positions{};
        std::size_t ticks{};
        Time::Duration remaining{};
    };

    const auto run_sequence = [] -> ReplayTrace {
        Time::FixedStepScheduler scheduler{kFixedStep};
        Time::BacklogPolicyController controller{Time::preserveBacklog};

        constexpr std::array<Time::Duration, 4> elapsed{
            kFourAndHalfSteps,
            Time::Duration::zero(),
            kTwoSteps,
            kHalfStep,
        };

        constexpr std::array<std::size_t, 4> budgets{
            kTwoUpdates,
            kOneUpdate,
            kOneUpdate,
            kOneUpdate,
        };

        ReplayTrace trace{};
        std::size_t position{};

        for (std::size_t i{}; i < elapsed.size(); ++i) {
            scheduler.accumulate(elapsed[i]);

            trace.results[i] = Time::runControlledFixedUpdateCycle(
                scheduler, budgets[i],
                [&](Time::Duration step) -> void {
                    EXPECT_EQ(step, kFixedStep);

                    ++trace.ticks;
                    ++position;

                    if (trace.ticks == 1U || trace.ticks == 3U) {
                        controller.requestPolicy(Time::discardExcessBacklog);
                    } else if (trace.ticks == 2U) {
                        controller.requestPolicy(Time::preserveBacklog);
                    }
                },
                controller);

            trace.positions[i] = position;
        }

        trace.remaining = scheduler.accumulatedTime();
        return trace;
    };

    const ReplayTrace original = run_sequence();
    const ReplayTrace replay = run_sequence();

    for (std::size_t i{}; i < original.results.size(); ++i) {
        EXPECT_EQ(original.results[i].updates_executed,
                  replay.results[i].updates_executed);
        EXPECT_EQ(original.results[i].time_discarded,
                  replay.results[i].time_discarded);
        EXPECT_EQ(original.results[i].remaining_time,
                  replay.results[i].remaining_time);
        EXPECT_EQ(original.results[i].overloaded, replay.results[i].overloaded);
        EXPECT_EQ(original.positions[i], replay.positions[i]);
    }

    EXPECT_EQ(original.ticks, 5U);
    EXPECT_EQ(original.positions[0], 2U);
    EXPECT_EQ(original.positions[1], 3U);
    EXPECT_EQ(original.positions[2], 4U);
    EXPECT_EQ(original.positions[3], 5U);

    EXPECT_EQ(original.results[0].time_discarded, Time::Duration::zero());
    EXPECT_EQ(original.results[1].time_discarded, Time::Duration::zero());
    EXPECT_EQ(original.results[2].time_discarded, kTwoSteps);
    EXPECT_TRUE(original.results[2].overloaded);

    EXPECT_EQ(original.ticks, replay.ticks);
    EXPECT_EQ(original.remaining, replay.remaining);
    EXPECT_EQ(original.remaining, Time::Duration::zero());
}

TEST(ControlledFixedUpdateCycleTests,
     DifferentPolicyRequestOrderCanProduceDifferentResults) {
    struct Outcome {
        Time::FixedUpdateCycleResult transition{};
        Time::FixedUpdateCycleResult following{};
        std::size_t ticks{};
    };

    const auto run_sequence = [](bool last_request_discards) -> Outcome {
        Time::FixedStepScheduler scheduler{kFixedStep};
        Time::BacklogPolicyController controller{Time::preserveBacklog};

        std::size_t ticks{};
        scheduler.accumulate(kFourAndHalfSteps);

        static_cast<void>(Time::runControlledFixedUpdateCycle(
            scheduler, kTwoUpdates,
            [&](Time::Duration step) -> void {
                EXPECT_EQ(step, kFixedStep);
                ++ticks;

                if (ticks == 1U) {
                    controller.requestPolicy(last_request_discards
                                                 ? Time::preserveBacklog
                                                 : Time::discardExcessBacklog);
                } else if (ticks == 2U) {
                    controller.requestPolicy(last_request_discards
                                                 ? Time::discardExcessBacklog
                                                 : Time::preserveBacklog);
                }
            },
            controller));

        const auto transition = Time::runControlledFixedUpdateCycle(
            scheduler, 0U, [](Time::Duration) -> void {}, controller);

        scheduler.accumulate(kHalfStep);

        const auto following = Time::runControlledFixedUpdateCycle(
            scheduler, kTwoUpdates, [&](Time::Duration) -> void { ++ticks; },
            controller);

        return {
            .transition = transition,
            .following = following,
            .ticks = ticks,
        };
    };

    const Outcome preserve_last = run_sequence(false);
    const Outcome discard_last = run_sequence(true);

    EXPECT_EQ(preserve_last.transition.time_discarded, Time::Duration::zero());
    EXPECT_EQ(preserve_last.transition.remaining_time, kTwoAndHalfSteps);

    EXPECT_EQ(discard_last.transition.time_discarded, kTwoSteps);
    EXPECT_EQ(discard_last.transition.remaining_time, kHalfStep);

    EXPECT_EQ(preserve_last.following.updates_executed, kTwoUpdates);
    EXPECT_EQ(discard_last.following.updates_executed, kOneUpdate);

    EXPECT_EQ(preserve_last.ticks, 4U);
    EXPECT_EQ(discard_last.ticks, 3U);

    EXPECT_TRUE(preserve_last.following.overloaded);
    EXPECT_FALSE(discard_last.following.overloaded);
}

TEST(ControlledFixedUpdateCycleTests,
     CustomPolicyTransitionPreservesOverloadDiagnostics) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    Time::BacklogPolicyController controller{Time::preserveBacklog};

    scheduler.accumulate(kFourAndHalfSteps);

    std::size_t policy_calls{};

    controller.requestPolicy(
        [&](const Time::BacklogContext &context) -> Time::BacklogDecision {
            ++policy_calls;

            EXPECT_EQ(context.fixed_step, kFixedStep);
            EXPECT_EQ(context.remaining_time, kTwoAndHalfSteps);
            EXPECT_EQ(context.update_budget, kTwoUpdates);
            EXPECT_EQ(context.updates_executed, kTwoUpdates);

            return Time::BacklogDecision{
                .time_to_discard = kFixedStep,
            };
        });

    const auto result = Time::runControlledFixedUpdateCycle(
        scheduler, kTwoUpdates, [](Time::Duration) -> void {}, controller);

    EXPECT_EQ(policy_calls, 1U);
    EXPECT_EQ(result.updates_executed, kTwoUpdates);
    EXPECT_EQ(result.time_discarded, kFixedStep);
    EXPECT_EQ(result.remaining_time, kOneAndHalfSteps);
    EXPECT_TRUE(result.overloaded);

    EXPECT_EQ(scheduler.accumulatedTime(), kOneAndHalfSteps);
    EXPECT_EQ(scheduler.fixedStep(), kFixedStep);
}

TEST(ControlledFixedUpdateCycleTests,
     RejectsInvalidCustomDecisionThroughController) {
    const auto run_invalid_cycle = [] -> void {
        Time::FixedStepScheduler scheduler{kFixedStep};

        Time::BacklogPolicyController controller{
            [](const Time::BacklogContext &) -> Time::BacklogDecision {
                return Time::BacklogDecision{
                    .time_to_discard = -kHalfStep,
                };
            }};

        scheduler.accumulate(kTwoAndHalfSteps);

        static_cast<void>(Time::runControlledFixedUpdateCycle(
            scheduler, 0U, [](Time::Duration) -> void {}, controller));
    };

    ASSERT_DEATH(run_invalid_cycle(),
                 "FixedStepScheduler requires discarded time to be finite, "
                 "nonnegative, and no greater than accumulated time");
}
