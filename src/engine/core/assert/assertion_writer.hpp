#pragma once

#include <iosfwd>

namespace SNE::Engine::Core::Assertion {
    class AssertionFailure;
    /**
     * @brief Writes an assertion failure as a formatted diagnostic.
     *
     * @param output_stream Stream that receives the diagnostic.
     * @param assertion_failure Assertion failure to write.
     */
    auto writeDiagnostic(std::ostream &output_stream,
                         const AssertionFailure &assertion_failure) -> void;
} // namespace SNE::Engine::Core::Assertion
