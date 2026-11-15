#pragma once

#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/core/time/backlog_policy_controller.hpp"
#include "engine/core/time/fixed_step_scheduler.hpp"
#include "engine/core/time/fixed_update_cycle.hpp"
#include <cstddef>

namespace SNE::Engine::Core::Time {
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
