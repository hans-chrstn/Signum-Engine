#pragma once

#include "engine/core/logging/log_record.hpp"
namespace SNE::Engine::Core::Logging {
    /**
     * @brief Abstract destination for structured engine log records.
     *
     * Defines the interface used by the logging system to deliver
     * diagnostic events to interchangeable output destinations.
     *
     * Implementations may write to terminals, files, or other
     * diagnostic consumers.
     *
     * Log sinks are responsible for consuming records, not for
     * determining event severity, creating timestamps, or
     * controlling application termination.
     *
     * @note Thread safety is coordinated by the logging system.
     * Individual sink implementations are not assumed to be
     * independently thread-safe.
     */
    class LogSink {
      public:
        /**
         * @brief Destroys the logging sink through its polymorphic interface.
         */
        virtual ~LogSink() = default;

        /**
         * @brief Consumes a structured logging event.
         *
         * The supplied record remains owned by the caller.
         * Implementations must copy any data they need to retain
         * after the call completes.
         *
         * @param record Logging event to consume.
         *
         * @throws Implementation-dependent exceptions if the sink
         * cannot process the record.
         */
        virtual auto write(const LogRecord &record) -> void = 0;

      private:
    };
} // namespace SNE::Engine::Core::Logging
