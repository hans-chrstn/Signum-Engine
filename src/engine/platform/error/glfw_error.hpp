#pragma once

#include "engine/core/error/native_error.hpp"

namespace SNE::Engine::Platform::Error {
    /**
     * @brief Captures the current GLFW error state.
     *
     * Retrieves GLFW's current error code and description and converts them
     * into the engine's native-error representation.
     *
     * Calling this function consumes GLFW's current error state.
     *
     * @return Native error information representing the current GLFW error.
     */
    [[nodiscard]] auto captureGlfwError() -> Core::Error::NativeError;
} // namespace SNE::Engine::Platform::Error
