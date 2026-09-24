#pragma once
#include "engine/core/error/engine_error.hpp"
namespace SNE::Engine::Core::Error {
    auto formatDiagnostic(const EngineError &engine_error) -> std::string;
} // namespace SNE::Engine::Core::Error
