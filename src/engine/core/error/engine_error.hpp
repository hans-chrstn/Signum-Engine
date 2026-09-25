#pragma once

#include "error_code.hpp"
#include "native_error.hpp"
#include "subsystem.hpp"

#include <optional>
#include <source_location>
#include <stdexcept>
#include <string>

namespace SNE::Engine::Core::Error {
    /**
     * @brief Represents a structured runtime failure produced by the engine.
     *
     * EngineError combines an engine-defined error code with a human-readable
     * message, the operation that failed, and the source location where the
     * error was created.
     *
     * An error may additionally contain NativeError information supplied by an
     * underlying API. The responsible engine subsystem is derived from the
     * engine error code.
     */
    class EngineError : public std::runtime_error {
      private:
        Code m_Code;
        std::optional<NativeError> m_NativeError;
        std::string m_Operation;
        std::source_location m_SourceLocation;

      public:
        /**
         * @brief Creates an engine error without native API error information.
         *
         * @param code Engine-defined error code.
         * @param msg Human-readable description of the failure.
         * @param operation Operation being performed when the failure occurred.
         * @param location Source location where the error was created.
         */
        EngineError(
            Code code, const char *msg, std::string operation = {},
            std::source_location location = std::source_location::current());

        /**
         * @brief Creates an engine error with native API error information.
         *
         * @param code Engine-defined error code.
         * @param msg Human-readable description of the failure.
         * @param nativeError Error information reported by the underlying API.
         * @param operation Operation being performed when the failure occurred.
         * @param location Source location where the error was created.
         */
        EngineError(
            Code code, const char *msg, NativeError nativeError,
            std::string operation = {},
            std::source_location location = std::source_location::current());

        /**
         * @brief Returns the engine-defined error code.
         *
         * @return Error code describing the failure.
         */
        [[nodiscard]] auto getCode() const noexcept -> Code;

        /**
         * @brief Returns optional error information from the underlying API.
         *
         * @return Native error information when available; otherwise an empty
         * optional.
         */
        [[nodiscard]] auto getNativeError() const noexcept
            -> const std::optional<NativeError> &;

        /**
         * @brief Returns the subsystem associated with this error.
         *
         * The subsystem is derived from the engine-defined error code.
         *
         * @return Engine subsystem responsible for the failure.
         */
        [[nodiscard]] auto getSubsystem() const noexcept -> Subsystem;

        /**
         * @brief Returns the operation being performed when the failure
         * occurred.
         *
         * @return Description of the failed operation.
         */
        [[nodiscard]] auto getOperation() const noexcept -> const std::string &;

        /**
         * @brief Returns the source location where the error was created.
         *
         * @return Captured source location of the failure.
         */
        [[nodiscard]] auto getSourceLocation() const noexcept
            -> std::source_location;
    };
} // namespace SNE::Engine::Core::Error
