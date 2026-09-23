#pragma once

#include <string>
namespace SNE::Engine::Core::Error {
    class NativeError {
      private:
        int m_NativeCode;
        std::string m_NativeDescription;

      public:
        NativeError(int nativeCode, std::string nativeDescription);
        [[nodiscard]] auto getNativeCode() const noexcept -> int;
        [[nodiscard]] auto getNativeDescription() const noexcept
            -> const std::string &;
    };
} // namespace SNE::Engine::Core::Error
