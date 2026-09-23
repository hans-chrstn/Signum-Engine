#pragma once

#include "error_code.hpp"
#include "native_error.hpp"
#include "subsystem.hpp"

#include <optional>
#include <stdexcept>
#include <string>

namespace SNE::Engine::Core::Error {
    class EngineError : public std::runtime_error {
      private:
        Code m_Code;
        std::optional<NativeError> m_NativeError;
        std::string m_Operation;

      public:
        EngineError(Code code, const char *msg, std::string operation = {});
        EngineError(Code code, const char *msg, NativeError nativeError,
                    std::string operation = {});
        [[nodiscard]] Code getCode() const noexcept;
        [[nodiscard]] const std::optional<NativeError> &
        getNativeError() const noexcept;
        [[nodiscard]] Subsystem getSubsystem() const noexcept;
        [[nodiscard]] const std::string &getOperation() const noexcept;
    };
} // namespace SNE::Engine::Core::Error
