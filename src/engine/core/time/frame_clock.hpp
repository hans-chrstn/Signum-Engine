#pragma once

#include <chrono>
#include <optional>

namespace SNE::Engine::Core::Time {
    /**
     * @brief Monotonic clock used for measuring elapsed time.
     */
    using Clock = std::chrono::steady_clock;

    /**
     * @brief Represents an elapsed duration in seconds.
     */
    using Duration = std::chrono::duration<double>;

    /**
     * @brief Measures elapsed time between consecutive frame timestamps.
     *
     * Tracks the previously sampled timestamp and computes the elapsed
     * duration between successive samples using a monotonic clock.
     *
     * The first sample establishes the initial timestamp and returns
     * a zero duration.
     *
     * Elapsed durations are reported without clamping or time scaling.
     * Consumers are responsible for applying their own timing policies.
     *
     * Timestamps are supplied explicitly to support deterministic testing
     * without depending on real-time execution or sleeping.
     *
     * @note This class is not thread-safe. Concurrent sampling requires
     * external synchronization.
     */
    class FrameClock {
      private:
        /**
         * @brief Timestamp recorded during the previous sample.
         *
         * Remains empty until the first sample establishes a baseline.
         * Subsequent samples use this timestamp to calculate elapsed time.
         */
        std::optional<Clock::time_point> m_PreviousTimestamp = std::nullopt;

      public:
        /**
         * @brief Measures elapsed time since the previous sample.
         *
         * Records the supplied timestamp and returns the duration
         * elapsed since the preceding sample.
         *
         * The first invocation returns zero because no previous
         * timestamp is available.
         *
         * @param now Current monotonic timestamp.
         * @return Elapsed duration in seconds, without clamping.
         *
         * @pre Timestamps must be supplied in nondecreasing order.
         *
         * @note Does not independently query the system clock.
         */
        [[nodiscard]] auto sample(Clock::time_point now) noexcept -> Duration;
    };
} // namespace SNE::Engine::Core::Time
