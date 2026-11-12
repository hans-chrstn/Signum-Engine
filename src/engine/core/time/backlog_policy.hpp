#pragma once

#include "engine/core/time/frame_clock.hpp"
#include <cstddef>

namespace SNE::Engine::Core::Time {
    /**
     * @brief Describes the state of a completed fixed-update scheduling cycle.
     *
     * Provides scheduling information for backlog policy evaluation
     * without exposing mutable scheduler state.
     *
     * Policies cannot modify the update budget or execute simulation ticks.
     */
    struct BacklogContext {
        /**
         * @brief Duration represented by one fixed simulation tick.
         *
         * Must be finite and strictly positive.
         */
        Duration fixed_step{};

        /**
         * @brief Unconsumed simulation time remaining after fixed updates.
         *
         * Includes complete pending ticks and any fractional remainder.
         * Must be finite and nonnegative.
         */
        Duration remaining_time{};

        /**
         * @brief Maximum number of fixed updates permitted during the cycle.
         *
         * May be zero to disable fixed updates for the current cycle.
         */
        std::size_t update_budget{};

        /**
         * @brief Number of fixed updates successfully completed during the
         * cycle.
         *
         * Must not exceed update_budget.
         */
        std::size_t updates_executed{};
    };

    /**
     * @brief Describes a backlog policy's requested time adjustment.
     *
     * Specifies how much unconsumed simulation time should be discarded.
     * The scheduling system validates and applies the decision.
     *
     * A zero duration preserves all remaining accumulated time.
     */
    struct BacklogDecision {
        /**
         * @brief Amount of pending simulation time requested for discard.
         *
         * Must be finite, nonnegative, and no greater than the
         * remaining accumulated time.
         */
        Duration time_to_discard{};
    };

    /**
     * @brief Preserves all accumulated simulation time.
     *
     * Returns a decision requesting no discarded time, regardless of
     * the scheduling context.
     *
     * @param context Read-only scheduling context.
     * @return Backlog decision with zero time requested for discard.
     */
    [[nodiscard]] auto preserveBacklog(const BacklogContext &context) noexcept
        -> BacklogDecision;

    /**
     * @brief Discards complete pending simulation steps.
     *
     * Requests removal of all complete fixed steps remaining after
     * a scheduling cycle while preserving the fractional remainder.
     *
     * Does not modify scheduler state or execute simulation updates.
     *
     * @param context Read-only scheduling context.
     * @return Decision specifying the complete-step time to discard.
     *
     * @pre The context contains a finite, strictly positive fixed step
     * and finite, nonnegative remaining time.
     */
    [[nodiscard]] auto
    discardExcessBacklog(const BacklogContext &context) noexcept
        -> BacklogDecision;
} // namespace SNE::Engine::Core::Time
