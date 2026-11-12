#include "engine/core/time/backlog_policy.hpp"
#include "engine/core/time/fixed_update_cycle.hpp"
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

    constexpr std::size_t kZeroUpdates{0};
    constexpr std::size_t kTwoUpdates{2};
    constexpr std::size_t kFiveUpdates{5};
} // namespace

TEST(FixedUpdateCycleTests, PreservePolicyRetainsBacklog) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    scheduler.accumulate(kThreeAndHalfSteps);

    std::size_t callback_count{};

    const Time::FixedUpdateCycleResult result = Time::runFixedUpdateCycle(
        scheduler, kTwoUpdates,
        [&](Time::Duration step) -> void {
            EXPECT_EQ(step, kFixedStep);
            ++callback_count;
        },
        Time::preserveBacklog);

    EXPECT_EQ(result.updates_executed, kTwoUpdates);
    EXPECT_EQ(result.time_discarded, Time::Duration::zero());
    EXPECT_EQ(result.remaining_time, kOneAndHalfSteps);
    EXPECT_EQ(scheduler.accumulatedTime(), kOneAndHalfSteps);
    EXPECT_EQ(callback_count, kTwoUpdates);
}

TEST(FixedUpdateCycleTests, DiscardExcessPreservesFractionalBacklog) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    scheduler.accumulate(kFourAndHalfSteps);

    std::size_t callback_count{};

    const Time::FixedUpdateCycleResult result = Time::runFixedUpdateCycle(
        scheduler, kTwoUpdates,
        [&](Time::Duration) -> void { ++callback_count; },
        Time::discardExcessBacklog);

    EXPECT_EQ(result.updates_executed, kTwoUpdates);
    EXPECT_EQ(result.time_discarded, kTwoSteps);
    EXPECT_EQ(result.remaining_time, kHalfStep);
    EXPECT_EQ(scheduler.accumulatedTime(), kHalfStep);
    EXPECT_EQ(callback_count, kTwoUpdates);
}

TEST(FixedUpdateCycleTests, ZeroBudgetPreservesBacklogWithPreservePolicy) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    scheduler.accumulate(kTwoAndHalfSteps);

    std::size_t callback_count{};

    const Time::FixedUpdateCycleResult result = Time::runFixedUpdateCycle(
        scheduler, kZeroUpdates,
        [&](Time::Duration) -> void { ++callback_count; },
        Time::preserveBacklog);

    EXPECT_EQ(result.updates_executed, kZeroUpdates);
    EXPECT_EQ(result.time_discarded, Time::Duration::zero());
    EXPECT_EQ(result.remaining_time, kTwoAndHalfSteps);
    EXPECT_EQ(callback_count, kZeroUpdates);
}

TEST(FixedUpdateCycleTests, CustomPolicyReceivesActualExecutedCount) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    scheduler.accumulate(kTwoAndHalfSteps);

    std::size_t policy_calls{};

    const Time::FixedUpdateCycleResult result = Time::runFixedUpdateCycle(
        scheduler, kFiveUpdates, [](Time::Duration) -> void {},
        [&](const Time::BacklogContext &context) -> Time::BacklogDecision {
            ++policy_calls;

            EXPECT_EQ(context.fixed_step, kFixedStep);
            EXPECT_EQ(context.remaining_time, kHalfStep);
            EXPECT_EQ(context.update_budget, kFiveUpdates);
            EXPECT_EQ(context.updates_executed, kTwoUpdates);

            return Time::BacklogDecision{};
        });

    EXPECT_EQ(policy_calls, 1U);
    EXPECT_EQ(result.updates_executed, kTwoUpdates);
    EXPECT_EQ(result.time_discarded, Time::Duration::zero());
    EXPECT_EQ(result.remaining_time, kHalfStep);
}

TEST(FixedUpdateCycleTests, CallbackFailureSkipsBacklogPolicy) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    scheduler.accumulate(kThreeAndHalfSteps);

    std::size_t callback_count{};
    std::size_t policy_calls{};

    EXPECT_THROW(
        static_cast<void>(Time::runFixedUpdateCycle(
            scheduler, kFiveUpdates,
            [&](Time::Duration) -> void {
                ++callback_count;

                if (callback_count == kTwoUpdates) {
                    throw std::runtime_error{"Simulation update failed"};
                }
            },
            [&](const Time::BacklogContext &) -> Time::BacklogDecision {
                ++policy_calls;
                return Time::BacklogDecision{};
            })),
        std::runtime_error);

    EXPECT_EQ(callback_count, kTwoUpdates);
    EXPECT_EQ(policy_calls, 0U);
    EXPECT_EQ(scheduler.accumulatedTime(), kOneAndHalfSteps);
}

TEST(FixedUpdateCycleTests, PolicyFailurePreservesRemainingTime) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    scheduler.accumulate(kThreeAndHalfSteps);

    std::size_t callback_count{};

    EXPECT_THROW(static_cast<void>(Time::runFixedUpdateCycle(
                     scheduler, kTwoUpdates,
                     [&](Time::Duration) -> void { ++callback_count; },
                     [](const Time::BacklogContext &) -> Time::BacklogDecision {
                         throw std::runtime_error{"Backlog policy failed"};
                     })),
                 std::runtime_error);

    EXPECT_EQ(callback_count, kTwoUpdates);
    EXPECT_EQ(scheduler.accumulatedTime(), kOneAndHalfSteps);
    EXPECT_EQ(scheduler.fixedStep(), kFixedStep);
}
