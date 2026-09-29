#pragma once

#include "assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include <source_location>
#include <string>

namespace SNE::Engine::Core::Assertion {
    /**
     * @brief Describes a programming assertion failure detected by the engine.
     *
     * Stores structured diagnostic information for a failed precondition,
     * invariant, or postcondition. Assertion failures represent programming
     * errors rather than runtime, environment, or native API failures.
     *
     * The type stores diagnostic information only. It does not represent a
     * recoverable error and does not participate in exception propagation.
     */
    class AssertionFailure {
      private:
        /** Category of programming assertion that failed. */
        AssertionType m_Type;
        /** Engine subsystem in which the assertion failed. */
        Error::Subsystem m_Subsystem;
        /** Human-readable description of the failed assertion. */
        std::string m_Message;
        /** Source location at which the assertion failure was created. */
        std::source_location m_SourceLocation;

      public:
        /**
         * @brief Creates structured information describing an assertion
         * failure.
         *
         * Captures the assertion category, responsible engine subsystem,
         * human-readable diagnostic message, and source location at which the
         * failure was detected.
         *
         * @param assertion_type Category of assertion that failed.
         * @param subsystem Engine subsystem in which the failure was detected.
         * @param message Human-readable description of the failed assertion.
         * @param source_location Source location at which the failure was
         * detected.
         */
        AssertionFailure(AssertionType assertion_type,
                         Error::Subsystem subsystem, std::string message,
                         std::source_location source_location =
                             std::source_location::current());

        /**
         * @brief Returns the category of assertion that failed.
         *
         * @return Assertion category associated with this failure.
         */
        [[nodiscard]] auto getType() const noexcept -> AssertionType;

        /**
         * @brief Returns the engine subsystem in which the assertion failed.
         *
         * @return Subsystem associated with this failure.
         */
        [[nodiscard]] auto getSubsystem() const noexcept -> Error::Subsystem;

        /**
         * @brief Returns the human-readable assertion failure message.
         *
         * The returned reference remains valid for the lifetime of this
         * AssertionFailure object.
         *
         * @return Read-only reference to the stored assertion message.
         */
        [[nodiscard]] auto getMessage() const noexcept -> const std::string &;

        /**
         * @brief Returns the source location where the assertion failure was
         * detected.
         *
         * @return Captured source location of the assertion failure.
         */
        [[nodiscard]] auto getSourceLocation() const noexcept
            -> std::source_location;
    };
} // namespace SNE::Engine::Core::Assertion
