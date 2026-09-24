#pragma once

#include <ostream>

namespace SNE::Engine::Core::Error {
    class EngineError;
    auto writeDiagnostic(std::ostream &output_stream,
                         const EngineError &engine_error) -> void;
} // namespace SNE::Engine::Core::Error
