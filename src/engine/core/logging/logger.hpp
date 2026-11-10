#pragma once

#include "engine/core/logging/log_level.hpp"
#include "engine/core/logging/log_record.hpp"
#include "engine/core/logging/log_sink.hpp"
#include <memory>
#include <mutex>
#include <queue>
#include <string>
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
     * @note Shared logging operations are synchronized internally.
     * Sink callbacks are serialized through a single active drainer.
     *
     * @warning The logger must outlive all concurrent operations
     * that access it. Callers must stop logging and wait for active
     * operations to finish before destroying the logger.
     *
     * @note Sink failures may leave pending records awaiting a
     * subsequent draining attempt. Explicit flushing and shutdown
     * failure policies are not yet implemented.
     */
    class Logger {
      private:
        /**
         * @brief Synchronizes access to the logger's shared state.
         *
         * Protects sink registration, severity configuration,
         * category configuration, and log dispatch coordination.
         *
         * Mutable to permit synchronization within const
         * member functions that read protected state.
         *
         * @note The locking strategy and reentrant logging policy
         * are defined by the logger's member implementations.
         */
        mutable std::mutex m_Mutex;

        /**
         * @brief Pending diagnostic records awaiting sink dispatch.
         *
         * Stores accepted log records in FIFO order until the active
         * draining thread delivers them to registered logging sinks.
         *
         * Records own their diagnostic data independently of callers.
         * Multiple threads may enqueue records, but only one thread
         * is permitted to drain the queue at a time.
         *
         * All queue operations must be synchronized using m_Mutex.
         */
        std::queue<LogRecord> m_PendingRecords;

        /**
         * @brief Indicates whether a thread is currently draining log records.
         *
         * Prevents multiple threads from concurrently dispatching records
         * to sinks that are not guaranteed to be thread-safe.
         *
         * Defaults to false when no thread owns dispatch responsibility.
         * Enqueuing threads may return while another thread is draining.
         *
         * Queue emptiness checks and changes to this flag must occur
         * atomically under m_Mutex to prevent abandoned records.
         *
         * Sink callbacks must execute without holding m_Mutex.
         */
        bool m_IsDraining{false};

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

        /**
         * @brief Owned category names permitted by the logger.
         *
         * An empty collection disables category filtering, allowing
         * events from every category that passes severity filtering.
         *
         * When populated, only events whose category exactly matches
         * a registered name remain eligible for sink dispatch.
         *
         * Category names are stored as owned strings so their
         * lifetimes remain independent of callers.
         *
         * Matching is case-sensitive.
         */
        std::vector<std::string> m_AllowedCategories;

        /**
         * @brief Delivers queued diagnostic records to registered sinks.
         *
         * Processes pending records in FIFO order while maintaining
         * exclusive responsibility for sink dispatch.
         *
         * Queue access and draining state transitions are synchronized
         * using m_Mutex. Sink callbacks execute without holding the
         * logger mutex, allowing other threads to enqueue records.
         *
         * The caller must have established exclusive draining
         * responsibility before invoking this function.
         *
         * @note Sink failure recovery must restore a valid draining
         * state without abandoning pending records.
         */
        auto drainPendingRecords() -> void;

      public:
        /**
         * @brief Constructs a logger without any registered output sinks.
         *
         * Initializes the logger with its default severity threshold
         * and unrestricted category filtering.
         *
         * Output destinations may be registered later using addSink().
         *
         * @note Accepted log records are discarded after dispatch
         * when no output sinks are registered.
         */
        Logger() = default;

        /**
         * @brief Constructs a logger with an initial output sink.
         *
         * Takes exclusive ownership of the supplied sink and registers
         * it before the logger becomes available for diagnostic events.
         *
         * Allows dependent engine systems to emit diagnostics during
         * their initialization without requiring separate sink registration.
         *
         * @param initial_sink Initial logging destination whose ownership
         * is transferred to the logger.
         *
         * @throws std::invalid_argument if the supplied sink is null.
         * @throws std::bad_alloc if sink registration requires allocation
         * and the allocation fails.
         *
         * @note Additional output sinks may be registered using addSink().
         */
        explicit Logger(std::unique_ptr<LogSink> initial_sink);

        /**
         * @brief Constructs a logger with multiple initial output sinks.
         *
         * Takes exclusive ownership of the supplied sinks and configures
         * the minimum accepted logging severity.
         *
         * All initial sinks are registered before the logger becomes
         * available to dependent application subsystems.
         *
         * An empty collection is permitted and results in a logger
         * without output destinations.
         *
         * @param initial_sinks Logging destinations whose ownership
         * is transferred to the logger.
         * @param minimum_level Minimum severity accepted by the logger.
         *
         * @throws std::invalid_argument if any supplied sink is null.
         *
         * @note Sink construction and configuration remain the
         * responsibility of the caller.
         *
         * @note No diagnostic records are emitted during construction.
         */
        Logger(std::vector<std::unique_ptr<LogSink>> initial_sinks,
               LogLevel minimum_level);

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
        auto setMinimumLevel(LogLevel level) -> void;

        /**
         * @brief Returns the minimum severity accepted by the logger.
         *
         * Provides read-only access to the current severity threshold
         * without exposing the logger's internal configuration.
         *
         * @return Currently configured minimum log severity.
         */
        [[nodiscard]] auto minimumLevel() const -> LogLevel;

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
         * @note Concurrent calls are synchronized internally.
         * Accepted records are queued and dispatched by a single
         * active draining thread.
         *
         * A call may return before its record reaches a sink when
         * another thread is already draining.
         *
         * Sink exceptions propagate to the active draining caller.
         * Other queued records may remain pending until a subsequent
         * logging operation resumes dispatch.
         */
        auto log(LogLevel level, std::string_view category,
                 std::string_view message) -> void;

        /**
         * @brief Adds a category to the logger's allowlist.
         *
         * Events are accepted from every category while the allowlist
         * is empty. Once a category is registered, only explicitly
         * allowed categories pass category filtering.
         *
         * Registering a category that already exists has no effect.
         * Category matching is exact and case-sensitive.
         *
         * The logger stores an owned copy of the supplied name.
         *
         * @param category Category name to permit.
         *
         * @throws std::bad_alloc if category storage allocation fails.
         *
         * @note Category registration is synchronized using the
         * logger mutex. Changes affect subsequent filter evaluations.
         */
        auto allowCategory(std::string_view category) -> void;

        /**
         * @brief Removes all category restrictions from the logger.
         *
         * Clears the category allowlist, restoring the default behavior
         * of accepting events from every category that satisfies
         * severity filtering.
         *
         * Previously registered category names are discarded.
         *
         * @note Category restrictions are modified under the logger
         * mutex. Changes affect subsequent filter evaluations.
         */
        auto clearAllowedCategories() -> void;
    };
} // namespace SNE::Engine::Core::Logging
