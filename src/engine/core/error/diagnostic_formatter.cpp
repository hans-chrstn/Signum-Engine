#include "diagnostic_formatter.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/subsystem.hpp"
#include <sstream>

namespace SNE::Engine::Core::Error {
    auto formatDiagnostic(const EngineError &engine_error) -> std::string {
        std::ostringstream diagnostic;
        const auto &nativeError = engine_error.getNativeError();
        diagnostic << "Engine Error" << "\n"
                   << "  Subsystem: " << toString(engine_error.getSubsystem())
                   << "\n"
                   << "  Operation: " << engine_error.getOperation() << "\n"
                   << "  Code: " << toString(engine_error.getCode()) << "\n"
                   << "  Message: " << engine_error.what() << "\n"
                   << "  Location: "
                   << engine_error.getSourceLocation().file_name() << ":"
                   << engine_error.getSourceLocation().line() << "\n"
                   << "  Function: "
                   << engine_error.getSourceLocation().function_name() << "\n";
        if (nativeError.has_value()) {
            diagnostic << "  Native Code: "
                       << nativeError.value().getNativeCode() << "\n"
                       << "  Native Info: "
                       << nativeError.value().getNativeDescription() << "\n";
        }
        return diagnostic.str();
    }
} // namespace SNE::Engine::Core::Error
