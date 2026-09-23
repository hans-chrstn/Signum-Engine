#include "engine/core/error/native_error.hpp"

namespace SNE::Engine::Platform::Error {
    [[nodiscard]] Core::Error::NativeError captureGlfwError();
} // namespace SNE::Engine::Platform::Error
