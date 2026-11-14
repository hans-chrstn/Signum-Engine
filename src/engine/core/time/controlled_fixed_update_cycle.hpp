#pragma once

#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/core/time/backlog_policy_controller.hpp"
#include "engine/core/time/fixed_step_scheduler.hpp"
#include "engine/core/time/fixed_update_cycle.hpp"
#include <cstddef>

namespace SNE::Engine::Core::Time {
    /**
     * @brief Executes a fixed-update cycle using a managed backlog policy.
     *
     * Activates pending policy changes at the beginning of the cycle,
     * then snapshots the active policy before executing simulation updates.
     *
     * Requests made during simulation callbacks remain pending until
     * a subsequent scheduling boundary.
     *
     * @param scheduler Scheduler owning accumulated simulation time.
     * @param max_updates Maximum fixed updates permitted during the cycle.
     * @param on_fixed_update Callback invoked for each fixed update.
     * @param controller Controller managing active and pending policies.
     * @return Completed fixed-update cycle results.
     *
     * @pre The controller must not already be executing a controlled cycle.
     *
     * @note Reentrant calls using the same controller fail a precondition
     * assertion. The execution guard is cleared on normal or exceptional exit.
     *
     * @note The selected policy is copied before simulation updates begin.
     * Copying the policy may throw without consuming simulation time.
     *
     * @note Simulation and policy exceptions propagate without automatic
     * rollback or retry.
     */
    template <typename UpdateFn>
    [[nodiscard]] auto runControlledFixedUpdateCycle(
        FixedStepScheduler &scheduler, std::size_t max_updates,
        UpdateFn &&on_fixed_update, BacklogPolicyController &controller)
        -> FixedUpdateCycleResult {
        if (controller.m_CycleInProgress) {
            Assertion::failAssertion(Assertion::AssertionType::Precondition,
                                     Error::Subsystem::Core,
                                     "Controlled fixed-update cycle requires "
                                     "no cycle to be in progress");
        }

        struct CycleCompletionGuard {
            bool &in_progress;

            ~CycleCompletionGuard() noexcept {
                in_progress = false;
            }
        };

        controller.m_CycleInProgress = true;

        CycleCompletionGuard guard{controller.m_CycleInProgress};

        static_cast<void>(controller.commitPendingPolicy());

        const BacklogPolicy policy{controller.activePolicy()};

        return runFixedUpdateCycle(scheduler, max_updates, on_fixed_update,
                                   policy);
    }
} // namespace SNE::Engine::Core::Time
