#include "engine_error.hpp"

namespace SNE::Engine::Core::Error {
    EngineError::EngineError(Code code, const char *msg)
        : std::runtime_error(msg), m_Code(code) {}

    EngineError::EngineError(Code code, const char *msg,
                             NativeError nativeError)
        : std::runtime_error(msg), m_Code(code),
          m_NativeError(std::move(nativeError)) {}

    Code EngineError::getCode() const noexcept {
        return m_Code;
    }

    const std::optional<NativeError> &
    EngineError::getNativeError() const noexcept {
        return m_NativeError;
    }
} // namespace SNE::Engine::Core::Error
