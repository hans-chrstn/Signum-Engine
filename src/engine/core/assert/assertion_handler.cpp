#include "assertion_handler.hpp"
#include "engine/core/assert/assertion_failure.hpp"
#include "engine/core/assert/assertion_writer.hpp"
#include <cstdlib>
#include <iostream>
#include <source_location>
#include <utility>

namespace SNE::Engine::Core::Assertion {
    auto failAssertion(AssertionType type, Error::Subsystem subsystem,
                       std::string message,
                       std::source_location source_location) -> void {
        const AssertionFailure assertion_failure = AssertionFailure(
            type, subsystem, std::move(message), source_location);

        writeDiagnostic(std::cerr, assertion_failure);

        std::abort();
    }
} // namespace SNE::Engine::Core::Assertion
