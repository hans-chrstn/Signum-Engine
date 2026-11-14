#pragma once

#include "engine/core/time/backlog_policy_application.hpp"
#include "engine/core/time/fixed_step_scheduler.hpp"
#include "engine/core/time/fixed_update_runner.hpp"
#include "engine/core/time/frame_clock.hpp"
#include <cstddef>
namespace SNE::Engine::Core::Time {
    /**
     * @brief Describes the outcome of a fixed-update scheduling cycle.
     *
     * Reports completed simulation updates, accumulated time discarded
     * by the selected backlog policy, time retained for subsequent cycles,
     * and whether scheduling overload occurred.
     *
     * Overload is detected before backlog policy application, so discarding
     * excess time does not erase the diagnostic.
     *
     * Does not own simulation state or modify scheduler behavior.
     */
    struct FixedUpdateCycleResult {
        /**
         * @brief Number of successfully completed fixed simulation updates.
         */
        std::size_t updates_executed{};

        /**
         * @brief Accumulated simulation time discarded by the backlog policy.
         */
        Duration time_discarded{};

        /**
         * @brief Unconsumed simulation time retained after policy application.
         */
        Duration remaining_time{};

        /**
         * @brief Indicates whether a complete simulation tick remained
         * unprocessed.
         *
         * True when at least one complete fixed timestep remains accumulated
         * after executing the permitted updates, but before applying the
         * selected backlog policy.
         *
         * Reports scheduling overload even when the policy subsequently
         * discards the excess accumulated time.
         */
        bool overloaded{};
    };

    /**
     * @brief Executes a complete fixed-update scheduling cycle.
     *
     * Runs available fixed simulation updates within the specified budget,
     * then evaluates and applies the selected backlog policy.
     *
     * Returns the number of successfully completed updates, the amount
     * of accumulated time discarded, the time retained for future
     * scheduling cycles, and whether a complete simulation tick remained
     * unprocessed before backlog policy application.
     *
     * Does not measure elapsed time, own simulation state, or perform
     * rendering operations.
     *
     * @param scheduler Scheduler owning the accumulated simulation time.
     * @param max_updates Maximum fixed updates permitted during this cycle.
     * @param on_fixed_update Callback invoked with the fixed-step duration
     * for each scheduled simulation update.
     * @param policy Callable accepting a BacklogContext and returning
     * a BacklogDecision.
     * @return Scheduling results containing executed updates, discarded
     * time, remaining accumulated time, and overload status.
     *
     * @note Fixed updates complete before the backlog policy is evaluated.
     * If an update callback throws, the exception propagates and the
     * backlog policy is not applied.
     *
     * @note Policy exceptions propagate without automatic retry.
     * Previously completed simulation updates are not rolled back.
     */
    template <typename UpdateFn, typename PolicyFn>
    [[nodiscard]] auto
    runFixedUpdateCycle(FixedStepScheduler &scheduler, std::size_t max_updates,
                        UpdateFn &&on_fixed_update, PolicyFn &&policy)
        -> FixedUpdateCycleResult {
        const std::size_t updates =
            runFixedUpdates(scheduler, max_updates, on_fixed_update);

        const bool overloaded =
            scheduler.accumulatedTime() >= scheduler.fixedStep();

        const Duration discarded_time =
            applyBacklogPolicy(scheduler, max_updates, updates, policy);

        const Duration remaining_time = scheduler.accumulatedTime();

        return FixedUpdateCycleResult{
            .updates_executed = updates,
            .time_discarded = discarded_time,
            .remaining_time = remaining_time,
            .overloaded = overloaded,
        };
    }
} // namespace SNE::Engine::Core::Time
