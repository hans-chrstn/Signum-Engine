#pragma once
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include <source_location>
#include <string>
namespace SNE::Engine::Core::Assertion {
    /**
     * @brief Reports an assertion failure and terminates execution.
     *
     * Creates a structured assertion failure from the supplied diagnostic
     * information, writes its diagnostic report, and terminates the process.
     *
     * @param type Type of assertion that failed.
     * @param subsystem Engine subsystem in which the failure occurred.
     * @param message Description of the violated assumption.
     * @param source_location Source location at which the failure was detected.
     */
    [[noreturn]] auto failAssertion(
        AssertionType type, Error::Subsystem subsystem, std::string message,
        std::source_location source_location = std::source_location::current())
        -> void;
} // namespace SNE::Engine::Core::Assertion
