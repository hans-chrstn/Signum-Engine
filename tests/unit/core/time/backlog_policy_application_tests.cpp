#include "engine/core/time/backlog_policy.hpp"
#include "engine/core/time/backlog_policy_application.hpp"
#include "engine/core/time/fixed_step_scheduler.hpp"
#include <cstddef>
#include <gtest/gtest.h>
#include <limits>
#include <stdexcept>

namespace Time = SNE::Engine::Core::Time;

namespace {
    constexpr Time::Duration kFixedStep{0.125};
    constexpr Time::Duration kHalfStep{0.0625};
    constexpr Time::Duration kTwoSteps{0.25};
    constexpr Time::Duration kTwoAndHalfSteps{0.3125};
    constexpr Time::Duration kThreeAndHalfSteps{0.4375};

    constexpr std::size_t kTwoUpdates{2};
    constexpr std::size_t kFiveUpdates{5};

    constexpr const char *kInvalidDiscardDiagnostic =
        "FixedStepScheduler requires discarded time to be finite, "
        "nonnegative, and no greater than accumulated time";
} // namespace

TEST(BacklogPolicyApplicationTests, PreserveRetainsAllAccumulatedTime) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    scheduler.accumulate(kThreeAndHalfSteps);

    const Time::Duration discarded = Time::applyBacklogPolicy(
        scheduler, kTwoUpdates, kTwoUpdates, Time::preserveBacklog);

    EXPECT_EQ(discarded, Time::Duration::zero());
    EXPECT_EQ(scheduler.accumulatedTime(), kThreeAndHalfSteps);
    EXPECT_EQ(scheduler.fixedStep(), kFixedStep);
}

TEST(BacklogPolicyApplicationTests,
     DiscardExcessRemovesCompleteStepsAndPreservesRemainder) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    scheduler.accumulate(kTwoAndHalfSteps);

    const Time::Duration discarded = Time::applyBacklogPolicy(
        scheduler, kTwoUpdates, kTwoUpdates, Time::discardExcessBacklog);

    EXPECT_EQ(discarded, kTwoSteps);
    EXPECT_EQ(scheduler.accumulatedTime(), kHalfStep);
    EXPECT_EQ(scheduler.fixedStep(), kFixedStep);
}

TEST(BacklogPolicyApplicationTests,
     CustomPolicyReceivesContextAndAppliesDecision) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    scheduler.accumulate(kTwoAndHalfSteps);

    std::size_t invocation_count{};

    const Time::Duration discarded = Time::applyBacklogPolicy(
        scheduler, kFiveUpdates, kTwoUpdates,
        [&](const Time::BacklogContext &context) -> Time::BacklogDecision {
            ++invocation_count;

            EXPECT_EQ(context.fixed_step, kFixedStep);
            EXPECT_EQ(context.remaining_time, kTwoAndHalfSteps);
            EXPECT_EQ(context.update_budget, kFiveUpdates);
            EXPECT_EQ(context.updates_executed, kTwoUpdates);

            return Time::BacklogDecision{
                .time_to_discard = kHalfStep,
            };
        });

    EXPECT_EQ(invocation_count, 1U);
    EXPECT_EQ(discarded, kHalfStep);
    EXPECT_EQ(scheduler.accumulatedTime(), kTwoSteps);
}

TEST(BacklogPolicyApplicationTests, RejectsExecutedUpdatesExceedingBudget) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    scheduler.accumulate(kTwoAndHalfSteps);

    ASSERT_DEATH(
        static_cast<void>(Time::applyBacklogPolicy(
            scheduler, kTwoUpdates, kFiveUpdates, Time::preserveBacklog)),
        "Backlog policy application requires completed updates not to "
        "exceed the update budget");
}

TEST(BacklogPolicyApplicationTests, RejectsNegativeCustomPolicyDecision) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    scheduler.accumulate(kTwoAndHalfSteps);

    ASSERT_DEATH(static_cast<void>(Time::applyBacklogPolicy(
                     scheduler, kTwoUpdates, kTwoUpdates,
                     [](const Time::BacklogContext &) -> Time::BacklogDecision {
                         return Time::BacklogDecision{
                             .time_to_discard = -kHalfStep,
                         };
                     })),
                 kInvalidDiscardDiagnostic);
}

TEST(BacklogPolicyApplicationTests, RejectsExcessiveCustomPolicyDecision) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    scheduler.accumulate(kTwoAndHalfSteps);

    ASSERT_DEATH(static_cast<void>(Time::applyBacklogPolicy(
                     scheduler, kTwoUpdates, kTwoUpdates,
                     [](const Time::BacklogContext &) -> Time::BacklogDecision {
                         return Time::BacklogDecision{
                             .time_to_discard = kThreeAndHalfSteps,
                         };
                     })),
                 kInvalidDiscardDiagnostic);
}

TEST(BacklogPolicyApplicationTests, RejectsInfiniteCustomPolicyDecision) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    scheduler.accumulate(kTwoAndHalfSteps);

    ASSERT_DEATH(
        static_cast<void>(Time::applyBacklogPolicy(
            scheduler, kTwoUpdates, kTwoUpdates,
            [](const Time::BacklogContext &) -> Time::BacklogDecision {
                return Time::BacklogDecision{
                    .time_to_discard =
                        Time::Duration{std::numeric_limits<double>::infinity()},
                };
            })),
        kInvalidDiscardDiagnostic);
}

TEST(BacklogPolicyApplicationTests, RejectsNaNCustomPolicyDecision) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    scheduler.accumulate(kTwoAndHalfSteps);

    ASSERT_DEATH(static_cast<void>(Time::applyBacklogPolicy(
                     scheduler, kTwoUpdates, kTwoUpdates,
                     [](const Time::BacklogContext &) -> Time::BacklogDecision {
                         return Time::BacklogDecision{
                             .time_to_discard =
                                 Time::Duration{
                                     std::numeric_limits<double>::quiet_NaN()},
                         };
                     })),
                 kInvalidDiscardDiagnostic);
}

TEST(BacklogPolicyApplicationTests,
     PolicyExceptionPropagatesWithoutDiscardingTime) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    scheduler.accumulate(kTwoAndHalfSteps);

    EXPECT_THROW(static_cast<void>(Time::applyBacklogPolicy(
                     scheduler, kTwoUpdates, kTwoUpdates,
                     [](const Time::BacklogContext &) -> Time::BacklogDecision {
                         throw std::runtime_error{
                             "Custom backlog policy failed"};
                     })),
                 std::runtime_error);

    EXPECT_EQ(scheduler.accumulatedTime(), kTwoAndHalfSteps);
    EXPECT_EQ(scheduler.fixedStep(), kFixedStep);
}
