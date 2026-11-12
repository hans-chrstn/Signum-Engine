#pragma once

#include "engine/core/time/fixed_step_scheduler.hpp"
#include <cstddef>

namespace SNE::Engine::Core::Time {
    /**
     * @brief Executes available fixed updates within a scheduling budget.
     *
     * Consumes at most max_updates fixed steps from the supplied scheduler,
     * invoking the update callback once for each successfully consumed step.
     *
     * Stops when the budget is exhausted or insufficient accumulated time
     * remains. Does not discard backlog or apply scheduling policies.
     *
     * @param scheduler Scheduler providing accumulated simulation time.
     * @param max_updates Maximum number of updates permitted in this cycle.
     * @param on_fixed_update Callback invoked with the fixed-step duration.
     * @return Number of successfully completed updates.
     *
     * @note Callback failures propagate to the caller. Automatic simulation
     * rollback and retry are not supported.
     */
    template <typename UpdateFn>
    [[nodiscard]] auto runFixedUpdates(FixedStepScheduler &scheduler,
                                       std::size_t max_updates,
                                       UpdateFn &&on_fixed_update)
        -> std::size_t {
        std::size_t counter{};

        while (counter < max_updates) {
            if (!scheduler.tryConsumeStep()) {
                break;
            }

            on_fixed_update(scheduler.fixedStep());

            ++counter;
        }

        return counter;
    }
} // namespace SNE::Engine::Core::Time
