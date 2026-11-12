#include "backlog_policy.hpp"
#include "engine/core/time/frame_clock.hpp"
#include <cmath>

namespace SNE::Engine::Core::Time {
    auto
    preserveBacklog([[maybe_unused]] const BacklogContext &context) noexcept
        -> BacklogDecision {
        return BacklogDecision{
            .time_to_discard = Duration{0},
        };
    }

    auto discardExcessBacklog(const BacklogContext &context) noexcept
        -> BacklogDecision {
        const double remainder = std::fmod(context.remaining_time.count(),
                                           context.fixed_step.count());

        const double discarded = context.remaining_time.count() - remainder;

        return BacklogDecision{
            .time_to_discard = Duration{discarded},
        };
    }
} // namespace SNE::Engine::Core::Time
