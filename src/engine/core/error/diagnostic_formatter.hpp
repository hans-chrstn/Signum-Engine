#pragma once
#include "engine/core/error/engine_error.hpp"
namespace SNE::Engine::Core::Error {
    /**
     * @brief Formats an engine error as a human-readable diagnostic report.
     *
     * The generated diagnostic includes the engine subsystem, failed operation,
     * error code, message, source location, and native error information when
     * available.
     *
     * @param engine_error Error to format.
     * @return Human-readable diagnostic report.
     */
    auto formatDiagnostic(const EngineError &engine_error) -> std::string;
} // namespace SNE::Engine::Core::Error
