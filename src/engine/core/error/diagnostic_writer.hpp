#pragma once

#include <ostream>

namespace SNE::Engine::Core::Error {
    class EngineError;
    /**
     * @brief Writes an engine error as a formatted diagnostic.
     *
     * @param output_stream Stream that receives the diagnostic.
     * @param engine_error Error to write.
     */
    auto writeDiagnostic(std::ostream &output_stream,
                         const EngineError &engine_error) -> void;
} // namespace SNE::Engine::Core::Error
