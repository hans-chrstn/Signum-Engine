#pragma once

#include "engine/core/logging/file_log_sink.hpp"
#include "engine/core/logging/log_level.hpp"
#include <filesystem>
#include <optional>

namespace SNE::Engine::Core::Logging {
    /**
     * @brief Configures diagnostic logging for an engine application.
     *
     * Defines the logging destinations and minimum severity used by
     * the application's logger.
     *
     * Terminal and file logging may be enabled independently, allowing
     * diagnostic events to be delivered to either or both destinations.
     *
     * The configuration is declarative and does not own logging sinks,
     * open files, or perform diagnostic dispatch.
     *
     * Logging destinations must be initialized before platform and
     * renderer subsystems that may emit startup diagnostics.
     *
     * @note An absent file path disables file logging.
     * @note Disabling every destination is permitted; accepted records
     * will have no output destination.
     * @note File rotation settings are validated when the file sink
     * is constructed, not when this configuration is created.
     */
    struct LoggingConfiguration {
        /**
         * @brief Whether diagnostic events are written to the terminal.
         *
         * When enabled, the application registers a terminal logging
         * sink with its logger.
         *
         * Defaults to true.
         */
        bool terminal_enabled{true};

        /**
         * @brief Optional filesystem destination for persistent log output.
         *
         * When present, the application constructs a file logging sink
         * using this path and the configured rotation policy.
         *
         * The destination directory must already exist.
         *
         * An absent value disables file logging.
         *
         * Defaults to std::nullopt.
         */
        std::optional<std::filesystem::path> file_path{std::nullopt};

        /**
         * @brief Rotation and retention settings for file logging.
         *
         * Controls the maximum active log file size and number of
         * archived log files retained during rotation.
         *
         * These settings are only used when file logging is enabled.
         *
         * Defaults to FileRotationPolicy's standard configuration.
         */
        FileRotationPolicy file_rotation_policy{};

        /**
         * @brief Minimum severity accepted by the application logger.
         *
         * Diagnostic events below this severity are discarded before
         * delivery to registered logging destinations.
         *
         * The threshold applies to all configured sinks.
         *
         * Defaults to LogLevel::Info.
         */
        LogLevel minimum_level{LogLevel::Info};
    };
} // namespace SNE::Engine::Core::Logging
