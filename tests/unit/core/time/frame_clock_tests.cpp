#include "engine/core/time/frame_clock.hpp"
#include <chrono>
#include <gtest/gtest.h>

namespace Time = SNE::Engine::Core::Time;

namespace {
    constexpr std::chrono::seconds kInitialTimestamp{10};

    constexpr std::chrono::milliseconds kFirstFrameDuration{16};
    constexpr std::chrono::milliseconds kSecondFrameDuration{17};
    constexpr std::chrono::milliseconds kLargeFrameDuration{500};

    constexpr Time::Clock::time_point kStartTime{kInitialTimestamp};
} // namespace

TEST(FrameClockTests, FirstSampleReturnsZero) {
    Time::FrameClock clock;

    const Time::Duration elapsed = clock.sample(kStartTime);

    EXPECT_DOUBLE_EQ(elapsed.count(), Time::Duration::zero().count());
}

TEST(FrameClockTests, MeasuresElapsedTimeBetweenSamples) {
    Time::FrameClock clock;

    static_cast<void>(clock.sample(kStartTime));

    const Time::Duration elapsed =
        clock.sample(kStartTime + kFirstFrameDuration);

    const Time::Duration expected{kFirstFrameDuration};

    EXPECT_DOUBLE_EQ(elapsed.count(), expected.count());
}

TEST(FrameClockTests, UpdatesPreviousTimestampAfterEachSample) {
    Time::FrameClock clock;

    static_cast<void>(clock.sample(kStartTime));

    const Time::Clock::time_point second_timestamp =
        kStartTime + kFirstFrameDuration;

    const Time::Clock::time_point third_timestamp =
        second_timestamp + kSecondFrameDuration;

    const Time::Duration first_elapsed = clock.sample(second_timestamp);
    const Time::Duration second_elapsed = clock.sample(third_timestamp);

    const Time::Duration expected_first{kFirstFrameDuration};
    const Time::Duration expected_second{kSecondFrameDuration};

    EXPECT_DOUBLE_EQ(first_elapsed.count(), expected_first.count());
    EXPECT_DOUBLE_EQ(second_elapsed.count(), expected_second.count());
}

TEST(FrameClockTests, EqualTimestampsProduceZeroElapsedTime) {
    Time::FrameClock clock;

    static_cast<void>(clock.sample(kStartTime));

    const Time::Duration elapsed = clock.sample(kStartTime);

    EXPECT_DOUBLE_EQ(elapsed.count(), Time::Duration::zero().count());
}

TEST(FrameClockTests, PreservesLargeElapsedDurationsWithoutClamping) {
    Time::FrameClock clock;

    static_cast<void>(clock.sample(kStartTime));

    const Time::Duration elapsed =
        clock.sample(kStartTime + kLargeFrameDuration);

    const Time::Duration expected{kLargeFrameDuration};

    EXPECT_DOUBLE_EQ(elapsed.count(), expected.count());
}

TEST(FrameClockTests, InstancesMaintainIndependentTimestamps) {
    Time::FrameClock first;
    Time::FrameClock second;

    static_cast<void>(first.sample(kStartTime));

    const Time::Clock::time_point later_timestamp =
        kStartTime + kFirstFrameDuration;

    const Time::Duration first_elapsed = first.sample(later_timestamp);
    const Time::Duration second_elapsed = second.sample(later_timestamp);

    const Time::Duration expected_first{kFirstFrameDuration};

    EXPECT_DOUBLE_EQ(first_elapsed.count(), expected_first.count());
    EXPECT_DOUBLE_EQ(second_elapsed.count(), Time::Duration::zero().count());
}
