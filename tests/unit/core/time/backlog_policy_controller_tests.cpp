#include "engine/core/time/backlog_policy.hpp"
#include "engine/core/time/backlog_policy_controller.hpp"
#include <cstddef>
#include <gtest/gtest.h>
#include <utility>

namespace Time = SNE::Engine::Core::Time;

namespace {
    constexpr const char *kInvalidPolicyDiagnostic =
        "BacklogPolicyController requires a valid initial policy";

    constexpr Time::Duration kFixedStep{0.125};
    constexpr Time::Duration kTwoAndHalfSteps{0.3125};

    constexpr Time::BacklogContext kPolicyContext{
        .fixed_step = kFixedStep,
        .remaining_time = kTwoAndHalfSteps,
        .update_budget = 2U,
        .updates_executed = 2U,
    };
} // namespace

TEST(BacklogPolicyControllerTests, AcceptsPreservePolicy) {
    EXPECT_NO_THROW(static_cast<void>(
        Time::BacklogPolicyController{Time::preserveBacklog}));
}

TEST(BacklogPolicyControllerTests, AcceptsDiscardExcessPolicy) {
    EXPECT_NO_THROW(static_cast<void>(
        Time::BacklogPolicyController{Time::discardExcessBacklog}));
}

TEST(BacklogPolicyControllerTests, AcceptsCustomPolicyWithoutInvokingIt) {
    std::size_t invocation_count{};

    Time::BacklogPolicy custom_policy =
        [&](const Time::BacklogContext &) -> Time::BacklogDecision {
        ++invocation_count;
        return Time::BacklogDecision{};
    };

    EXPECT_NO_THROW(static_cast<void>(
        Time::BacklogPolicyController{std::move(custom_policy)}));

    EXPECT_EQ(invocation_count, 0U);
}

TEST(BacklogPolicyControllerTests, RejectsEmptyInitialPolicy) {
    ASSERT_DEATH(
        static_cast<void>(Time::BacklogPolicyController{Time::BacklogPolicy{}}),
        kInvalidPolicyDiagnostic);
}

TEST(BacklogPolicyControllerTests, AcceptsValidPolicyRequest) {
    Time::BacklogPolicyController controller{Time::preserveBacklog};

    EXPECT_NO_THROW(controller.requestPolicy(Time::discardExcessBacklog));
}

TEST(BacklogPolicyControllerTests, AllowsReplacingPendingPolicy) {
    Time::BacklogPolicyController controller{Time::preserveBacklog};

    EXPECT_NO_THROW(controller.requestPolicy(Time::discardExcessBacklog));
    EXPECT_NO_THROW(controller.requestPolicy(Time::preserveBacklog));
}

TEST(BacklogPolicyControllerTests, RequestDoesNotInvokePendingPolicy) {
    Time::BacklogPolicyController controller{Time::preserveBacklog};

    std::size_t invocation_count{};

    controller.requestPolicy(
        [&](const Time::BacklogContext &) -> Time::BacklogDecision {
            ++invocation_count;
            return Time::BacklogDecision{};
        });

    EXPECT_EQ(invocation_count, 0U);
}

TEST(BacklogPolicyControllerTests, RejectsEmptyRequestedPolicy) {
    Time::BacklogPolicyController controller{Time::preserveBacklog};

    ASSERT_DEATH(controller.requestPolicy(Time::BacklogPolicy{}),
                 "BacklogPolicyController requires a valid requested policy");
}

TEST(BacklogPolicyControllerTests, ExposesInitialActivePolicy) {
    const Time::BacklogPolicyController controller{Time::preserveBacklog};

    const Time::BacklogDecision decision =
        controller.activePolicy()(kPolicyContext);

    EXPECT_EQ(decision.time_to_discard, Time::Duration::zero());
}

TEST(BacklogPolicyControllerTests, RequestDoesNotChangeActivePolicy) {
    Time::BacklogPolicyController controller{Time::preserveBacklog};

    controller.requestPolicy(Time::discardExcessBacklog);

    const Time::BacklogDecision decision =
        controller.activePolicy()(kPolicyContext);

    EXPECT_EQ(decision.time_to_discard, Time::Duration::zero());
}
