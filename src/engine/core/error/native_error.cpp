#include "native_error.hpp"
#include <utility>

namespace SNE::Engine::Core::Error {
    NativeError::NativeError(int nativeCode, std::string nativeDescription)
        : m_NativeCode(nativeCode),
          m_NativeDescription(std::move(nativeDescription)) {}

    auto NativeError::getNativeCode() const noexcept -> int {
        return m_NativeCode;
    }

    auto NativeError::getNativeDescription() const noexcept
        -> const std::string & {
        return m_NativeDescription;
    }
} // namespace SNE::Engine::Core::Error
