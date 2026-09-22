#pragma once

#include <string>
namespace SNE::Engine::Core::Error {
    class NativeError {
      private:
        int m_NativeCode;
        std::string m_NativeDescription;

      public:
        NativeError(int nativeCode, std::string nativeDescription);
        [[nodiscard]] int getNativeCode() const noexcept;
        [[nodiscard]] const std::string &getNativeDescription() const noexcept;
    };
} // namespace SNE::Engine::Core::Error
