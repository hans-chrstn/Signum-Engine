#pragma once

#include "engine/core/error/native_error.hpp"

namespace SNE::Engine::Platform::Error {
    [[nodiscard]] auto captureGlfwError() -> Core::Error::NativeError;
} // namespace SNE::Engine::Platform::Error
