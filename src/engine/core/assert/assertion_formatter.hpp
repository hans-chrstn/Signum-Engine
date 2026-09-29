#pragma once
#include "assertion_failure.hpp"
#include <string>

namespace SNE::Engine::Core::Assertion {
    /**
     * @brief Formats an assertion failure as a human-readable diagnostic
     * report.
     *
     * The generated diagnostic includes the assertion type, engine subsystem,
     * failure message, source location, and function name.
     *
     * @param assertion_failure Assertion failure to format.
     * @return Human-readable diagnostic report.
     */
    auto formatDiagnostic(const AssertionFailure &assertion_failure)
        -> std::string;
} // namespace SNE::Engine::Core::Assertion
