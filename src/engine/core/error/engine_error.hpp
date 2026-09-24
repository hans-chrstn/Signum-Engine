#pragma once

#include "error_code.hpp"
#include "native_error.hpp"
#include "subsystem.hpp"

#include <optional>
#include <source_location>
#include <stdexcept>
#include <string>

namespace SNE::Engine::Core::Error {
    class EngineError : public std::runtime_error {
      private:
        Code m_Code;
        std::optional<NativeError> m_NativeError;
        std::string m_Operation;
        std::source_location m_SourceLocation;

      public:
        EngineError(
            Code code, const char *msg, std::string operation = {},
            std::source_location location = std::source_location::current());
        EngineError(
            Code code, const char *msg, NativeError nativeError,
            std::string operation = {},
            std::source_location location = std::source_location::current());
        [[nodiscard]] auto getCode() const noexcept -> Code;
        [[nodiscard]] auto getNativeError() const noexcept
            -> const std::optional<NativeError> &;
        [[nodiscard]] auto getSubsystem() const noexcept -> Subsystem;
        [[nodiscard]] auto getOperation() const noexcept -> const std::string &;
        [[nodiscard]] auto getSourceLocation() const noexcept
            -> std::source_location;
    };
} // namespace SNE::Engine::Core::Error
