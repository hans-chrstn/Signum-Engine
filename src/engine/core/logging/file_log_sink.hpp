#pragma once

#include "engine/core/logging/log_record.hpp"
#include "engine/core/logging/log_sink.hpp"
#include "engine/core/logging/stream_log_sink.hpp"
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>

namespace SNE::Engine::Core::Logging {
    /**
     * @brief Configures size-based rotation and retention of log files.
     *
     * Defines the maximum size of an active log file and the number
     * of archived log files retained during rotation.
     *
     * When the active file reaches its configured size threshold,
     * the logging system rotates the file and manages older archives
     * according to this policy.
     *
     * @note A maximum archive count of zero disables archive retention.
     * @note The maximum file size must be greater than zero.
     * @note Policy validation is performed by FileLogSink.
     */
    struct FileRotationPolicy {
        /**
         * @brief Default maximum active log file size in bytes.
         *
         * Equivalent to 10 MiB (10,485,760 bytes).
         */
        static constexpr std::uintmax_t kDefaultMaxFileSizeBytes =
            10ULL * 1024ULL * 1024ULL;

        /**
         * @brief Maximum active log file size before rotation.
         *
         * Expressed in bytes. Must be greater than zero.
         *
         * Defaults to 10 MiB.
         */
        std::uintmax_t max_file_size_bytes{kDefaultMaxFileSizeBytes};

        /**
         * @brief Maximum number of archived log files to retain.
         *
         * Older archives are removed when this limit is exceeded.
         *
         * Zero disables archive retention.
         *
         * Defaults to three archived files.
         */
        std::size_t max_archives{3};
    };

    /**
     * @brief Writes structured logging events to an owned log file.
     *
     * Provides a file-backed logging destination using the existing
     * stream logging implementation.
     *
     * Owns the output file stream and delegates record formatting
     * and writing to StreamLogSink.
     *
     * File ownership remains internal to this sink. The underlying
     * stream must remain valid for the lifetime of m_StreamSink.
     *
     * @note Thread safety is coordinated by Logger. Concurrent
     * calls to write() are not independently synchronized.
     *
     * @note File opening and writing failures must be reported
     * rather than silently discarded.
     */
    class FileLogSink final : public LogSink {
      private:
        /**
         * @brief Filesystem path of the active log file.
         *
         * Retains an owned copy of the path supplied during construction.
         *
         * Used to open the active log file and derive archive paths
         * when performing size-based rotation.
         *
         * The path remains unchanged throughout the sink's lifetime.
         */
        std::filesystem::path m_Path;

        /**
         * @brief Owned output file stream.
         *
         * Manages the underlying file resource through RAII.
         *
         * Must be declared before m_StreamSink so that the file
         * remains alive while the stream sink references it.
         *
         * The file is closed automatically during destruction.
         */
        std::ofstream m_File;

        /**
         * @brief Stream-based logging implementation.
         *
         * Borrows m_File and delegates record formatting to the
         * existing logging formatter.
         *
         * Does not own or close the underlying file.
         *
         * Must be destroyed before m_File.
         */
        StreamLogSink m_StreamSink;

        /**
         * @brief Rotation and retention configuration for this file sink.
         *
         * Stores the maximum active file size and the number of archived
         * files retained during rotation.
         *
         * The policy is copied during construction and remains owned
         * by this sink.
         */
        FileRotationPolicy m_RotationPolicy;

        /**
         * @brief Current logical size of the active log file in bytes.
         *
         * Initialized from the existing file size when the sink opens.
         *
         * Updated after successful writes so rotation can detect when
         * the configured size threshold has been reached.
         *
         * Assumes this sink exclusively manages writes to the active file.
         */
        std::uintmax_t m_CurrentFileSize{0};

        /**
         * @brief Indicates that a rotation has not completed successfully.
         *
         * Set before beginning rotation and cleared only after the active
         * log file has been successfully reopened and its size reset.
         *
         * Remains true if rotation fails, preventing subsequent writes
         * from using a potentially inconsistent logging destination.
         */
        bool m_RotationIncomplete{false};

        /**
         * @brief Constructs the filesystem path of a rotated log archive.
         *
         * Derives an archive path from the active log file path by appending
         * a numeric suffix to its filename.
         *
         * Archive indices begin at one, with lower indices representing
         * newer archives.
         *
         * For example, an active log file named "signum.log" produces
         * "signum.log.1" for index one and "signum.log.2" for index two.
         *
         * The original active log file path remains unchanged.
         *
         * @param index One-based archive index to append to the filename.
         *
         * @return Filesystem path identifying the requested log archive.
         *
         * @pre index must be greater than zero.
         *
         * @throws std::bad_alloc if path or string construction requires
         * memory allocation that fails.
         *
         * @note This function constructs a path only. It does not create,
         * rename, remove, or otherwise modify filesystem entries.
         */
        [[nodiscard]] auto archivePath(std::size_t index) const
            -> std::filesystem::path;

        /**
         * @brief Rotates the active log file and manages retained archives.
         *
         * Closes the active file, applies the configured archive retention
         * policy, and opens a fresh active log file.
         *
         * Existing archives are shifted toward higher indices, with the
         * oldest archive removed when the retention limit is reached.
         *
         * When archive retention is disabled, the active file is replaced
         * without preserving an archived copy.
         *
         * @throws Error::EngineError if closing, rotating, or reopening
         * the log file fails.
         *
         * @note This operation must not write diagnostics through the
         * logging system because it manages the logging destination.
         */
        auto rotate() -> void;

      public:
        /**
         * @brief Opens a file-backed logging destination with rotation.
         *
         * Initializes the owned file stream and configures size-based
         * log rotation using the supplied policy.
         *
         * Existing log data is preserved until rotation requires
         * replacing or removing archived files.
         *
         * @param path Filesystem path to the active log file.
         * @param rotation_policy Configuration controlling file size
         * thresholds and archive retention.
         *
         * @throws std::invalid_argument if the rotation policy is invalid.
         * @throws An exception if the log file cannot be opened.
         *
         * @note Uses the default rotation policy when none is supplied.
         */
        explicit FileLogSink(std::filesystem::path path,
                             FileRotationPolicy rotation_policy = {});

        /**
         * @brief Writes a formatted logging event to the file.
         *
         * Delegates formatting and output to m_StreamSink.
         *
         * @param record Structured logging event to write.
         *
         * @throws Exceptions from formatting or file-output failures.
         *
         * @note The supplied record remains owned by the caller.
         */
        auto write(const LogRecord &record) -> void override;
    };
} // namespace SNE::Engine::Core::Logging
