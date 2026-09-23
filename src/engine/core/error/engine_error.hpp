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
        [[nodiscard]] auto getCode() const noexcept -> Code;
        [[nodiscard]] auto getNativeError() const noexcept
            -> const std::optional<NativeError> &;
        [[nodiscard]] auto getSubsystem() const noexcept -> Subsystem;
        [[nodiscard]] auto getOperation() const noexcept -> const std::string &;
    };
} // namespace SNE::Engine::Core::Error
