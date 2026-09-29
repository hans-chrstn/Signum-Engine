#include "assertion_formatter.hpp"
#include "assertion_failure.hpp"
#include "assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include <source_location>
#include <sstream>

namespace SNE::Engine::Core::Assertion {
    auto formatDiagnostic(const AssertionFailure &assertion_failure)
        -> std::string {
        std::ostringstream diagnostic;
        const std::source_location source_location =
            assertion_failure.getSourceLocation();
        diagnostic << "Assertion Failure" << '\n'
                   << "  Type: " << toString(assertion_failure.getType())
                   << '\n'
                   << "  Subsystem: "
                   << Error::toString(assertion_failure.getSubsystem()) << '\n'
                   << "  Message: " << assertion_failure.getMessage() << '\n'
                   << "  Location: " << source_location.file_name() << ":"
                   << source_location.line() << '\n'
                   << "  Function: " << source_location.function_name() << '\n';

        return diagnostic.str();
    }
} // namespace SNE::Engine::Core::Assertion
