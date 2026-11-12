#include "engine/core/time/backlog_policy.hpp"
#include <cstddef>
#include <gtest/gtest.h>

namespace Time = SNE::Engine::Core::Time;

namespace {
    constexpr Time::Duration kFixedStep{0.125};
    constexpr Time::Duration kRemainingTime{0.3125};

    constexpr std::size_t kZeroUpdates{0};
    constexpr std::size_t kTwoUpdates{2};
    constexpr std::size_t kFiveUpdates{5};

    constexpr Time::Duration kHalfStep{0.0625};
    constexpr Time::Duration kOneStep{0.125};
    constexpr Time::Duration kTwoSteps{0.25};
    constexpr Time::Duration kTwoAndHalfSteps{0.3125};
} // namespace

TEST(BacklogPolicyTests, PreserveBacklogDiscardsNothingWhenEmpty) {
    const Time::BacklogContext context{
        .fixed_step = kFixedStep,
        .remaining_time = Time::Duration::zero(),
        .update_budget = kFiveUpdates,
        .updates_executed = kZeroUpdates,
    };

    const Time::BacklogDecision decision = Time::preserveBacklog(context);

    EXPECT_EQ(decision.time_to_discard, Time::Duration::zero());
}

TEST(BacklogPolicyTests, PreserveBacklogRetainsAllPendingTime) {
    const Time::BacklogContext context{
        .fixed_step = kFixedStep,
        .remaining_time = kRemainingTime,
        .update_budget = kFiveUpdates,
        .updates_executed = kTwoUpdates,
    };

    const Time::BacklogDecision decision = Time::preserveBacklog(context);

    EXPECT_EQ(decision.time_to_discard, Time::Duration::zero());
}

TEST(BacklogPolicyTests, PreserveBacklogIgnoresExhaustedBudget) {
    const Time::BacklogContext context{
        .fixed_step = kFixedStep,
        .remaining_time = kRemainingTime,
        .update_budget = kTwoUpdates,
        .updates_executed = kTwoUpdates,
    };

    const Time::BacklogDecision decision = Time::preserveBacklog(context);

    EXPECT_EQ(decision.time_to_discard, Time::Duration::zero());
}

TEST(BacklogPolicyTests, DiscardExcessPreservesEmptyBacklog) {
    const Time::BacklogContext context{
        .fixed_step = kFixedStep,
        .remaining_time = Time::Duration::zero(),
        .update_budget = kFiveUpdates,
        .updates_executed = kZeroUpdates,
    };

    const Time::BacklogDecision decision = Time::discardExcessBacklog(context);

    EXPECT_EQ(decision.time_to_discard, Time::Duration::zero());
}

TEST(BacklogPolicyTests, DiscardExcessPreservesFractionalTime) {
    const Time::BacklogContext context{
        .fixed_step = kFixedStep,
        .remaining_time = kHalfStep,
        .update_budget = kFiveUpdates,
        .updates_executed = kTwoUpdates,
    };

    const Time::BacklogDecision decision = Time::discardExcessBacklog(context);

    EXPECT_EQ(decision.time_to_discard, Time::Duration::zero());
}

TEST(BacklogPolicyTests, DiscardExcessRemovesExactlyOneStep) {
    const Time::BacklogContext context{
        .fixed_step = kFixedStep,
        .remaining_time = kOneStep,
        .update_budget = kTwoUpdates,
        .updates_executed = kTwoUpdates,
    };

    const Time::BacklogDecision decision = Time::discardExcessBacklog(context);

    EXPECT_EQ(decision.time_to_discard, kOneStep);
}

TEST(BacklogPolicyTests, DiscardExcessRemovesMultipleCompleteSteps) {
    const Time::BacklogContext context{
        .fixed_step = kFixedStep,
        .remaining_time = kTwoSteps,
        .update_budget = kTwoUpdates,
        .updates_executed = kTwoUpdates,
    };

    const Time::BacklogDecision decision = Time::discardExcessBacklog(context);

    EXPECT_EQ(decision.time_to_discard, kTwoSteps);
}

TEST(BacklogPolicyTests, DiscardExcessRetainsRemainder) {
    const Time::BacklogContext context{
        .fixed_step = kFixedStep,
        .remaining_time = kTwoAndHalfSteps,
        .update_budget = kTwoUpdates,
        .updates_executed = kTwoUpdates,
    };

    const Time::BacklogDecision decision = Time::discardExcessBacklog(context);

    EXPECT_EQ(decision.time_to_discard, kTwoSteps);
    EXPECT_EQ(context.remaining_time - decision.time_to_discard, kHalfStep);
}
