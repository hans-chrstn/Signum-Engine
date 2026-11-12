#include "fixed_step_scheduler.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/core/time/frame_clock.hpp"
#include <cmath>

namespace SNE::Engine::Core::Time {
    FixedStepScheduler::FixedStepScheduler(Duration fixed_step)
        : m_FixedStep(fixed_step) {
        if (fixed_step <= Duration::zero() ||
            !std::isfinite(fixed_step.count())) {
            Assertion::failAssertion(Assertion::AssertionType::Precondition,
                                     Error::Subsystem::Core,
                                     "FixedStepScheduler requires a finite, "
                                     "strictly positive fixed-step duration");
        }
    }

    auto FixedStepScheduler::accumulate(Duration elapsed) -> void {
        if (elapsed < Duration::zero() || !std::isfinite(elapsed.count())) {
            Assertion::failAssertion(Assertion::AssertionType::Precondition,
                                     Error::Subsystem::Core,
                                     "FixedStepScheduler requires a finite, "
                                     "nonnegative elapsed duration");
        }

        const Duration candidate = m_AccumulatedTime + elapsed;

        if (candidate < Duration::zero() || !std::isfinite(candidate.count())) {
            Assertion::failAssertion(Assertion::AssertionType::Precondition,
                                     Error::Subsystem::Core,
                                     "FixedStepScheduler accumulated duration "
                                     "exceeds its finite range");
        }

        m_AccumulatedTime = candidate;
    }

    auto FixedStepScheduler::accumulatedTime() const noexcept -> Duration {
        return m_AccumulatedTime;
    }

    auto FixedStepScheduler::tryConsumeStep() noexcept -> bool {
        if (m_AccumulatedTime < m_FixedStep) {
            return false;
        }

        m_AccumulatedTime -= m_FixedStep;

        return true;
    }
} // namespace SNE::Engine::Core::Time
