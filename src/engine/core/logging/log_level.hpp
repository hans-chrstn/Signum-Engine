#pragma once

#include <cstdint>
#include <string_view>

namespace SNE::Engine::Core::Logging {
    /**
     * @brief Identifies the severity of an engine log event.
     *
     * Levels are ordered from the least severe (Trace)
     * to the most severe (Fatal).
     *
     * Used for log filtering and diagnostic classification.
     * A log level does not itself determine error recovery
     * or application termination behavior.
     */
    enum class LogLevel : std::uint8_t {
        /** Detailed execution information used for fine-grained diagnostics. */
        Trace,

        /** Development diagnostics useful for debugging engine behavior. */
        Debug,

        /** Informational messages describing normal engine operations. */
        Info,

        /** Potential problems that do not necessarily prevent execution. */
        Warning,

        /** Failures that prevent an operation from completing successfully. */
        Error,

        /**
         * Critical failures that may require application shutdown.
         *
         * This severity does not itself terminate the application.
         * Fatal error handling remains the responsibility of the
         * established error boundary.
         */
        Fatal,
    };

    /**
     * @brief Returns the symbolic name of a log severity level.
     *
     * Converts a valid LogLevel enumerator to its corresponding
     * human-readable name.
     *
     * @param level Log severity level to convert.
     * @return Symbolic name of the level, or "Unknown" for
     * an unrecognized value.
     */
    [[nodiscard]] auto toString(LogLevel level) noexcept -> std::string_view;
} // namespace SNE::Engine::Core::Logging
