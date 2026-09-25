#pragma once

#include <string>
namespace SNE::Engine::Core::Error {
    /**
     * @brief Stores error information reported by an underlying API.
     *
     * NativeError preserves both the numeric error value and human-readable
     * description supplied by an external library or system API.
     */
    class NativeError {
      private:
        int m_NativeCode;
        std::string m_NativeDescription;

      public:
        /**
         * @brief Creates native error information.
         *
         * @param nativeCode Numeric error value reported by the originating
         * API.
         * @param nativeDescription Human-readable description reported by the
         * originating API.
         */
        NativeError(int nativeCode, std::string nativeDescription);
        /**
         * @brief Returns the native error code.
         *
         * @return Numeric error value reported by the originating API.
         */
        [[nodiscard]] auto getNativeCode() const noexcept -> int;
        /**
         * @brief Returns the native error description.
         *
         * @return Human-readable description reported by the originating API.
         */
        [[nodiscard]] auto getNativeDescription() const noexcept
            -> const std::string &;
    };
} // namespace SNE::Engine::Core::Error
