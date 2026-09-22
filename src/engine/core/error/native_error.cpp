#include "native_error.hpp"
#include <string>
#include <utility>

namespace SNE::Engine::Core::Error {
    NativeError::NativeError(int nativeCode, std::string nativeDescription)
        : m_NativeCode(nativeCode),
          m_NativeDescription(std::move(nativeDescription)) {}

    int NativeError::getNativeCode() const noexcept {
        return m_NativeCode;
    }

    const std::string &NativeError::getNativeDescription() const noexcept {
        return m_NativeDescription;
    }
} // namespace SNE::Engine::Core::Error
