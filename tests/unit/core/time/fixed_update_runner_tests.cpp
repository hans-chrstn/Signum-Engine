#include "engine/core/time/fixed_step_scheduler.hpp"
#include "engine/core/time/fixed_update_runner.hpp"
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

    constexpr std::size_t kZeroUpdates{0};
    constexpr std::size_t kTwoUpdates{2};
    constexpr std::size_t kFiveUpdates{5};
} // namespace

TEST(FixedUpdateRunnerTests, ZeroBudgetExecutesNoUpdates) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    scheduler.accumulate(kThreeAndHalfSteps);

    std::size_t callback_count{};

    const std::size_t executed = Time::runFixedUpdates(
        scheduler, kZeroUpdates,
        [&](Time::Duration) -> void { ++callback_count; });

    EXPECT_EQ(executed, kZeroUpdates);
    EXPECT_EQ(callback_count, kZeroUpdates);
    EXPECT_EQ(scheduler.accumulatedTime(), kThreeAndHalfSteps);
}

TEST(FixedUpdateRunnerTests, NoAvailableTimeExecutesNoUpdates) {
    Time::FixedStepScheduler scheduler{kFixedStep};

    std::size_t callback_count{};

    const std::size_t executed = Time::runFixedUpdates(
        scheduler, kFiveUpdates,
        [&](Time::Duration) -> void { ++callback_count; });

    EXPECT_EQ(executed, kZeroUpdates);
    EXPECT_EQ(callback_count, kZeroUpdates);
    EXPECT_EQ(scheduler.accumulatedTime(), Time::Duration::zero());
}

TEST(FixedUpdateRunnerTests, RespectsMaximumUpdateBudget) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    scheduler.accumulate(kThreeAndHalfSteps);

    std::size_t callback_count{};

    const std::size_t executed = Time::runFixedUpdates(
        scheduler, kTwoUpdates, [&](Time::Duration fixed_step) -> void {
            EXPECT_EQ(fixed_step, kFixedStep);
            ++callback_count;
        });

    EXPECT_EQ(executed, kTwoUpdates);
    EXPECT_EQ(callback_count, kTwoUpdates);
    EXPECT_EQ(scheduler.accumulatedTime(), kOneAndHalfSteps);
}

TEST(FixedUpdateRunnerTests, StopsWhenAvailableTicksAreExhausted) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    scheduler.accumulate(kTwoAndHalfSteps);

    std::size_t callback_count{};

    const std::size_t executed = Time::runFixedUpdates(
        scheduler, kFiveUpdates,
        [&](Time::Duration) -> void { ++callback_count; });

    EXPECT_EQ(executed, kTwoUpdates);
    EXPECT_EQ(callback_count, kTwoUpdates);
    EXPECT_EQ(scheduler.accumulatedTime(), kHalfStep);
}

TEST(FixedUpdateRunnerTests, PropagatesCallbackFailureWithoutRetry) {
    Time::FixedStepScheduler scheduler{kFixedStep};
    scheduler.accumulate(kThreeAndHalfSteps);

    std::size_t callback_count{};

    EXPECT_THROW(static_cast<void>(Time::runFixedUpdates(
                     scheduler, kFiveUpdates,
                     [&](Time::Duration) -> void {
                         ++callback_count;

                         if (callback_count == kTwoUpdates) {
                             throw std::runtime_error{
                                 "Simulation update failed"};
                         }
                     })),
                 std::runtime_error);

    EXPECT_EQ(callback_count, kTwoUpdates);
    EXPECT_EQ(scheduler.accumulatedTime(), kOneAndHalfSteps);
}
