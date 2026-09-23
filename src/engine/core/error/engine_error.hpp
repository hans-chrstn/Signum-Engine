#pragma once

#include "error_code.hpp"
#include "native_error.hpp"
#include "subsystem.hpp"

#include <optional>
#include <stdexcept>

namespace SNE::Engine::Core::Error {
    class EngineError : public std::runtime_error {
      private:
        Code m_Code;
        std::optional<NativeError> m_NativeError;

      public:
        EngineError(Code code, const char *msg);
        EngineError(Code code, const char *msg, NativeError nativeError);
        [[nodiscard]] Code getCode() const noexcept;
        [[nodiscard]] const std::optional<NativeError> &
        getNativeError() const noexcept;
        [[nodiscard]] Subsystem getSubsystem() const noexcept;
    };
} // namespace SNE::Engine::Core::Error
