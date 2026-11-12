#pragma once

#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/core/time/backlog_policy.hpp"
#include "engine/core/time/fixed_step_scheduler.hpp"
#include "engine/core/time/frame_clock.hpp"
#include <cstddef>

namespace SNE::Engine::Core::Time {
    /**
     * @brief Evaluates and applies a backlog policy after fixed updates.
     *
     * Constructs a read-only scheduling context from the scheduler's
     * current state, invokes the supplied policy, and applies the
     * requested time discard through the scheduler.
     *
     * Does not execute simulation updates or modify the fixed timestep.
     *
     * @param scheduler Scheduler owning the accumulated simulation time.
     * @param update_budget Maximum updates permitted during the cycle.
     * @param updates_executed Successfully completed updates.
     * @param policy Callable accepting a BacklogContext and returning
     * a BacklogDecision.
     * @return Amount of accumulated time discarded.
     *
     * @pre updates_executed must not exceed update_budget.
     *
     * @note The policy must not modify scheduler state during evaluation.
     * Policy exceptions propagate without automatic retry.
     */
    template <typename PolicyFn>
    [[nodiscard]] auto applyBacklogPolicy(FixedStepScheduler &scheduler,
                                          std::size_t update_budget,
                                          std::size_t updates_executed,
                                          PolicyFn &&policy) -> Duration {
        if (updates_executed > update_budget) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition, Error::Subsystem::Core,
                "Backlog policy application requires completed updates not to "
                "exceed the update budget");
        }

        const BacklogContext context{
            .fixed_step = scheduler.fixedStep(),
            .remaining_time = scheduler.accumulatedTime(),
            .update_budget = update_budget,
            .updates_executed = updates_executed,
        };

        const BacklogDecision decision = policy(context);
        scheduler.discardAccumulatedTime(decision.time_to_discard);

        return decision.time_to_discard;
    }
} // namespace SNE::Engine::Core::Time
