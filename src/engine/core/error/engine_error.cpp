#include "engine_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include <utility>

namespace SNE::Engine::Core::Error {

    EngineError::EngineError(Code code, const char *msg, std::string operation)
        : std::runtime_error(msg), m_Code(code),
          m_Operation(std::move(operation)) {}

    EngineError::EngineError(Code code, const char *msg,
                             NativeError nativeError, std::string operation)
        : std::runtime_error(msg), m_Code(code),
          m_NativeError(std::move(nativeError)),
          m_Operation(std::move(operation)) {}

    Code EngineError::getCode() const noexcept {
        return m_Code;
    }

    const std::optional<NativeError> &
    EngineError::getNativeError() const noexcept {
        return m_NativeError;
    }

    Subsystem EngineError::getSubsystem() const noexcept {
        return getSubsystemFor(m_Code);
    }

    const std::string &EngineError::getOperation() const noexcept {
        return m_Operation;
    }
} // namespace SNE::Engine::Core::Error
