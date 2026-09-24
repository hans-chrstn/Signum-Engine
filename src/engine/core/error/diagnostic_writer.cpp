#include "diagnostic_writer.hpp"
#include "diagnostic_formatter.hpp"

namespace SNE::Engine::Core::Error {
    auto writeDiagnostic(std::ostream &output_stream,
                         const EngineError &engine_error) -> void {
        output_stream << formatDiagnostic(engine_error);
    }
} // namespace SNE::Engine::Core::Error
