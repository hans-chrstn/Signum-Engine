#include "assertion_writer.hpp"
#include "assertion_formatter.hpp"

namespace SNE::Engine::Core::Assertion {
    auto writeDiagnostic(std::ostream &output_stream,
                         const AssertionFailure &assertion_failure) -> void {
        output_stream << formatDiagnostic(assertion_failure);
    }
} // namespace SNE::Engine::Core::Assertion
