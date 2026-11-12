#pragma once

#include "engine/core/time/frame_clock.hpp"

namespace SNE::Engine::Core::Time {
    /**
     * @brief Accumulates elapsed time for fixed-step simulation scheduling.
     *
     * Maintains a configured fixed timestep and accumulated elapsed time
     * to support simulation updates at a consistent temporal interval,
     * independently of variable-rate rendering.
     *
     * Scheduling limits and backlog handling policies determine how
     * accumulated time is processed without changing the fixed timestep.
     *
     * Does not own or execute simulation systems, measure wall-clock time,
     * or depend on a rendering backend.
     *
     * @note This class is not thread-safe. Concurrent access requires
     * external synchronization.
     */
    class FixedStepScheduler {
      private:
        /**
         * @brief Duration represented by one fixed simulation update.
         *
         * Must be finite and strictly positive.
         */
        Duration m_FixedStep{};

        /**
         * @brief Elapsed time retained for future simulation updates.
         *
         * Initially zero. Accumulated time is consumed as fixed simulation
         * updates are scheduled, subject to the configured backlog policy.
         */
        Duration m_AccumulatedTime{};

      public:
        /**
         * @brief Constructs a scheduler with the specified fixed timestep.
         *
         * Initializes the accumulated time to zero.
         *
         * @param fixed_step Duration of one simulation update.
         *
         * @pre fixed_step must be finite and strictly positive.
         */
        explicit FixedStepScheduler(Duration fixed_step);

        /**
         * @brief Adds elapsed time to the pending simulation accumulator.
         *
         * Preserves previously accumulated time without executing or
         * consuming simulation ticks.
         *
         * @param elapsed Nonnegative, finite elapsed duration to accumulate.
         *
         * @pre elapsed must be finite and nonnegative.
         * @pre Adding elapsed must not overflow the duration representation.
         */
        auto accumulate(Duration elapsed) -> void;

        /**
         * @brief Returns elapsed time retained for future simulation updates.
         *
         * Includes both complete pending simulation steps and any fractional
         * remainder that has not yet been consumed.
         *
         * @return Currently accumulated, unconsumed elapsed duration.
         */
        [[nodiscard]] auto accumulatedTime() const noexcept -> Duration;

        /**
         * @brief Attempts to consume one fixed simulation timestep.
         *
         * Subtracts the configured fixed-step duration from accumulated
         * time when sufficient time is available.
         *
         * Leaves accumulated time unchanged when less than one complete
         * fixed timestep remains.
         *
         * Does not execute simulation logic or apply backlog policies.
         *
         * @return True if one fixed step was consumed; otherwise false.
         */
        [[nodiscard]] auto tryConsumeStep() noexcept -> bool;

        /**
         * @brief Returns the configured fixed simulation timestep.
         *
         * This duration represents the amount of simulation time
         * advanced by each successfully consumed fixed step.
         *
         * @return Finite, strictly positive fixed-step duration.
         */
        [[nodiscard]] auto fixedStep() const noexcept -> Duration;
    };
} // namespace SNE::Engine::Core::Time
