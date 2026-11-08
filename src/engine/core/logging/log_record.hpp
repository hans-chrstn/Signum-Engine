#pragma once

#include "engine/core/logging/log_level.hpp"
#include <chrono>
#include <string>

namespace SNE::Engine::Core::Logging {
    /**
     * @brief Represents a structured diagnostic event emitted by the engine.
     *
     * Stores the severity, category, and message associated with a
     * single logging event.
     *
     * Category and message strings are owned by the record so that
     * their lifetimes remain independent of the original caller.
     *
     * Log records contain diagnostic data independently of how
     * individual logging sinks format or present that data.
     */
    struct LogRecord {
        /**
         * @brief Severity assigned to the logging event.
         *
         * Determines the event's importance for severity-based filtering.
         */
        LogLevel level;

        /**
         * @brief Logical category identifying the source of the event.
         *
         * Categories may identify an engine subsystem or a more
         * specific operation, such as renderer shader compilation.
         */
        std::string category;

        /**
         * @brief Human-readable description of the logging event.
         *
         * Owns the message text independently of the caller.
         */
        std::string message;

        /**
         * @brief Wall-clock time at which the logging event occurred.
         *
         * Captured when the event is emitted, independently of when
         * a logging sink eventually processes or displays the record.
         */
        std::chrono::system_clock::time_point timestamp;
    };
} // namespace SNE::Engine::Core::Logging
