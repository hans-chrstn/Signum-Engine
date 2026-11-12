#include "frame_clock.hpp"

namespace SNE::Engine::Core::Time {
    auto FrameClock::sample(Clock::time_point now) noexcept -> Duration {
        const Duration elapsed_time = now - m_PreviousTimestamp.value_or(now);

        m_PreviousTimestamp = now;

        return elapsed_time;
    }
} // namespace SNE::Engine::Core::Time
