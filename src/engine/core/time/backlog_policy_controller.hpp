#pragma once

#include "engine/core/time/backlog_policy.hpp"
#include <cstddef>
#include <functional>
#include <optional>

namespace SNE::Engine::Core::Time {
    class FixedStepScheduler;
    struct FixedUpdateCycleResult;
    class BacklogPolicyController;

    template <typename UpdateFn>
    [[nodiscard]] auto runControlledFixedUpdateCycle(
        FixedStepScheduler &scheduler, std::size_t max_updates,
        UpdateFn &&on_fixed_update, BacklogPolicyController &controller)
        -> FixedUpdateCycleResult;

    using BacklogPolicy =
        std::function<BacklogDecision(const BacklogContext &)>;

    /**
     * @brief Manages active and pending backlog policy selections.
     *
     * Retains the active policy while replacement requests await a
     * scheduling boundary.
     *
     * Does not own the fixed-step scheduler, execute simulation updates,
     * or modify accumulated simulation time.
     *
     * @note This class is not thread-safe. Concurrent access requires
     * external synchronization.
     */
    class BacklogPolicyController {
      private:
        /**
         * @brief Policy selected for the current scheduling cycle.
         *
         * Must contain a valid callable after construction.
         */
        BacklogPolicy m_Active;

        /**
         * @brief Requested replacement awaiting activation.
         *
         * An empty optional indicates that no change is pending.
         */
        std::optional<BacklogPolicy> m_Pending = std::nullopt;

        /**
         * @brief Indicates whether a controlled scheduling cycle is executing.
         *
         * Prevents nested controlled cycles from activating pending policies
         * while the current cycle is still executing.
         *
         * Must be reset when the cycle finishes, including on exceptions.
         */
        bool m_CycleInProgress = false;

        template <typename UpdateFn>
        friend auto runControlledFixedUpdateCycle(
            FixedStepScheduler &scheduler, std::size_t max_updates,
            UpdateFn &&on_fixed_update, BacklogPolicyController &controller)
            -> FixedUpdateCycleResult;

        /**
         * @brief Activates a pending backlog policy replacement.
         *
         * Replaces the active policy with the pending policy when a
         * replacement is available, then clears the pending request.
         *
         * Leaves the active policy unchanged when no replacement is pending.
         *
         * Does not modify accumulated simulation time or execute updates.
         *
         * @return True if a pending policy was activated; otherwise false.
         *
         * @note Must be called at a safe scheduling boundary, not while
         * the active policy is being evaluated.
         */
        [[nodiscard]] auto commitPendingPolicy() noexcept -> bool;

      public:
        /**
         * @brief Constructs a controller with an initial active backlog policy.
         *
         * Initializes the active policy from the supplied callable.
         * No replacement policy is initially pending.
         *
         * @param initial_policy Initial policy used for scheduling cycles.
         *
         * @pre initial_policy must contain a valid callable.
         */
        explicit BacklogPolicyController(BacklogPolicy initial_policy);

        /**
         * @brief Requests a backlog policy replacement.
         *
         * Stores the supplied policy as a pending replacement without
         * changing the currently active policy.
         *
         * If another replacement is already pending, the newer request
         * supersedes it.
         *
         * The pending policy becomes active only when explicitly committed
         * at a scheduling boundary.
         *
         * @param policy Replacement backlog policy to queue.
         *
         * @pre policy must contain a valid callable.
         */
        auto requestPolicy(BacklogPolicy policy) -> void;

        /**
         * @brief Returns the currently active backlog policy.
         *
         * Provides read-only access to the selected policy without copying
         * the stored callable or activating pending replacements.
         *
         * @return Reference to the active backlog policy.
         *
         * @note The returned reference is owned by this controller.
         * It must not outlive the controller. A policy commit replaces
         * the callable referenced by this accessor.
         */
        [[nodiscard]] auto activePolicy() const noexcept
            -> const BacklogPolicy &;
    };
} // namespace SNE::Engine::Core::Time
