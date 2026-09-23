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

    auto EngineError::getCode() const noexcept -> Code {
        return m_Code;
    }

    auto EngineError::getNativeError() const noexcept
        -> const std::optional<NativeError> & {
        return m_NativeError;
    }

    auto EngineError::getSubsystem() const noexcept -> Subsystem {
        return getSubsystemFor(m_Code);
    }

    auto EngineError::getOperation() const noexcept -> const std::string & {
        return m_Operation;
    }
} // namespace SNE::Engine::Core::Error
