#include "engine_error.hpp"

namespace SNE::Engine::Core::Error {
    EngineError::EngineError(Code code, const char *msg)
        : std::runtime_error(msg), m_Code(code) {}

    Code EngineError::getCode() const noexcept {
        return m_Code;
    }
} // namespace SNE::Engine::Core::Error
