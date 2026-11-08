#pragma once

#include "engine/core/logging/log_level.hpp"
#include "engine/core/logging/log_sink.hpp"
#include <memory>
#include <string_view>
#include <vector>

namespace SNE::Engine::Core::Logging {
    /**
     * @brief Coordinates structured logging across registered output sinks.
     *
     * Owns the logging sinks responsible for consuming diagnostic
     * events produced by engine subsystems.
     *
     * The logger will apply event filtering, capture event timestamps,
     * and dispatch accepted records to its registered sinks.
     *
     * Sink objects are exclusively owned by the logger, while any
     * external resources borrowed by individual sinks must remain
     * valid for the duration of their use.
     *
     * Logging remains independent of editor functionality and does
     * not control application termination.
     *
     * @note Thread safety and logging failure handling will be
     * established as the logging implementation develops.
     */
    class Logger {
      private:
        /**
         * @brief Exclusively owned destinations for accepted log records.
         *
         * Each sink is destroyed automatically when its owning
         * smart pointer is destroyed.
         *
         * The logger controls sink registration and lifetime.
         * Individual sinks may borrow external resources whose
         * lifetimes must be managed separately.
         */
        std::vector<std::unique_ptr<LogSink>> m_Sinks;

        /**
         * @brief Minimum severity accepted by the logger.
         *
         * Events below this level are discarded before sink dispatch.
         * Defaults to Info to suppress verbose tracing and debugging
         * messages during normal operation.
         *
         * This setting controls diagnostic visibility, not application
         * termination or error recovery.
         */
        LogLevel m_MinimumLevel{LogLevel::Info};

      public:
        /**
         * @brief Registers a logging sink and takes exclusive ownership of it.
         *
         * The registered sink receives accepted logging events until
         * the logger is destroyed or the sink is explicitly removed.
         *
         * The supplied sink must not be null. Ownership is transferred
         * from the caller to the logger.
         *
         * @param sink Logging destination whose ownership is transferred.
         *
         * @throws std::invalid_argument if the supplied sink is null.
         * @throws std::bad_alloc if sink storage allocation fails.
         */
        auto addSink(std::unique_ptr<LogSink> sink) -> void;

        /**
         * @brief Configures the minimum severity accepted by the logger.
         *
         * Events below this threshold will be discarded before sink
         * dispatch. Events at or above the threshold remain eligible
         * for delivery.
         *
         * Changing the threshold does not affect application termination
         * or error recovery behavior.
         *
         * @param level New minimum accepted log severity.
         */
        auto setMinimumLevel(LogLevel level) noexcept -> void;

        /**
         * @brief Returns the minimum severity accepted by the logger.
         *
         * Provides read-only access to the current severity threshold
         * without exposing the logger's internal configuration.
         *
         * @return Currently configured minimum log severity.
         */
        [[nodiscard]] auto minimumLevel() const noexcept -> LogLevel;

        /**
         * @brief Emits a structured diagnostic event to registered sinks.
         *
         * Discards events below the configured minimum severity.
         *
         * Accepted events receive an emission timestamp and are
         * dispatched to all registered logging sinks.
         *
         * Category and message are borrowed only for the duration
         * of this call. Accepted records own copies of their text.
         *
         * @param level Severity assigned to the event.
         * @param category Logical category identifying the event source.
         * @param message Human-readable diagnostic message.
         *
         * @throws Exceptions propagated from record construction
         * or registered logging sinks.
         *
         * @note Concurrent logging and sink failure handling will be
         * addressed separately.
         */
        auto log(LogLevel level, std::string_view category,
                 std::string_view message) -> void;
    };
} // namespace SNE::Engine::Core::Logging
