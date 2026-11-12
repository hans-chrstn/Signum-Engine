#include "engine/core/time/fixed_step_scheduler.hpp"
#include <chrono>
#include <gtest/gtest.h>
#include <limits>

namespace Time = SNE::Engine::Core::Time;

namespace {
    constexpr double kValidStepSeconds = 1.0 / 60.0;
    constexpr double kNegativeStepSeconds = -0.01;

    constexpr Time::Duration kValidStep{kValidStepSeconds};

    constexpr const char *kInvalidStepDiagnostic =
        "FixedStepScheduler requires a finite, "
        "strictly positive fixed-step duration";

    constexpr std::chrono::milliseconds kFixedStep{10};
    constexpr std::chrono::milliseconds kFirstElapsed{6};
    constexpr std::chrono::milliseconds kSecondElapsed{8};

    constexpr Time::Duration kAccumulatorStep{kFixedStep};
} // namespace

TEST(FixedStepSchedulerTests, AcceptsValidFixedStep) {
    EXPECT_NO_THROW(static_cast<void>(Time::FixedStepScheduler{kValidStep}));
}

TEST(FixedStepSchedulerTests, RejectsZeroFixedStep) {
    ASSERT_DEATH(
        static_cast<void>(Time::FixedStepScheduler{Time::Duration::zero()}),
        kInvalidStepDiagnostic);
}

TEST(FixedStepSchedulerTests, RejectsNegativeFixedStep) {
    ASSERT_DEATH(static_cast<void>(Time::FixedStepScheduler{
                     Time::Duration{kNegativeStepSeconds}}),
                 kInvalidStepDiagnostic);
}

TEST(FixedStepSchedulerTests, RejectsInfiniteFixedStep) {
    ASSERT_DEATH(static_cast<void>(Time::FixedStepScheduler{
                     Time::Duration{std::numeric_limits<double>::infinity()}}),
                 kInvalidStepDiagnostic);
}

TEST(FixedStepSchedulerTests, RejectsNaNFixedStep) {
    ASSERT_DEATH(static_cast<void>(Time::FixedStepScheduler{
                     Time::Duration{std::numeric_limits<double>::quiet_NaN()}}),
                 kInvalidStepDiagnostic);
}

TEST(FixedStepSchedulerTests, AccumulatedTimeStartsAtZero) {
    Time::FixedStepScheduler scheduler{kAccumulatorStep};

    EXPECT_EQ(scheduler.accumulatedTime(), Time::Duration::zero());
}

TEST(FixedStepSchedulerTests, AccumulatesElapsedTimeAcrossCalls) {
    Time::FixedStepScheduler scheduler{kAccumulatorStep};

    scheduler.accumulate(Time::Duration{kFirstElapsed});
    scheduler.accumulate(Time::Duration{kSecondElapsed});

    const Time::Duration expected{kFirstElapsed + kSecondElapsed};

    EXPECT_DOUBLE_EQ(scheduler.accumulatedTime().count(), expected.count());
}

TEST(FixedStepSchedulerTests, AcceptsZeroElapsedTime) {
    Time::FixedStepScheduler scheduler{kAccumulatorStep};

    scheduler.accumulate(Time::Duration{kFirstElapsed});
    scheduler.accumulate(Time::Duration::zero());

    const Time::Duration expected{kFirstElapsed};

    EXPECT_DOUBLE_EQ(scheduler.accumulatedTime().count(), expected.count());
}

TEST(FixedStepSchedulerTests, RejectsNegativeElapsedTime) {
    Time::FixedStepScheduler scheduler{kAccumulatorStep};

    ASSERT_DEATH(scheduler.accumulate(Time::Duration{kNegativeStepSeconds}),
                 "requires a finite, nonnegative elapsed duration");
}

TEST(FixedStepSchedulerTests, RejectsInfiniteElapsedTime) {
    Time::FixedStepScheduler scheduler{kAccumulatorStep};

    ASSERT_DEATH(scheduler.accumulate(
                     Time::Duration{std::numeric_limits<double>::infinity()}),
                 "requires a finite, nonnegative elapsed duration");
}

TEST(FixedStepSchedulerTests, RejectsNaNElapsedTime) {
    Time::FixedStepScheduler scheduler{kAccumulatorStep};

    ASSERT_DEATH(scheduler.accumulate(
                     Time::Duration{std::numeric_limits<double>::quiet_NaN()}),
                 "requires a finite, nonnegative elapsed duration");
}

TEST(FixedStepSchedulerTests, RejectsAccumulationOverflow) {
    Time::FixedStepScheduler scheduler{kAccumulatorStep};

    const Time::Duration maximum{std::numeric_limits<double>::max()};

    scheduler.accumulate(maximum);

    ASSERT_DEATH(scheduler.accumulate(maximum),
                 "accumulated duration exceeds its finite range");
}
