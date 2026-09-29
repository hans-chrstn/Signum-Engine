#pragma once

#include "assertion_failure.hpp"
#include <ostream>

namespace SNE::Engine::Core::Assertion {
    /**
     * @brief Writes an assertion failure as a formatted diagnostic.
     *
     * @param output_stream Stream that receives the diagnostic.
     * @param assertion_failure Assertion failure to write.
     */
    auto writeDiagnostic(std::ostream &output_stream,
                         const AssertionFailure &assertion_failure) -> void;
} // namespace SNE::Engine::Core::Assertion
