#pragma once

#include "error_code.hpp"
#include "native_error.hpp"
#include <optional>
#include <stdexcept>
#include <string>

namespace SNE::Engine::Core::Error {
    class EngineError : public std::runtime_error {
      private:
        Code m_Code;
        NativeError m_NativeError;

      public:
        EngineError(Code code, const char *msg);
        EngineError(Code code, const char *msg, NativeError);
        [[nodiscard]] Code getCode() const noexcept;
    };
} // namespace SNE::Engine::Core::Error
