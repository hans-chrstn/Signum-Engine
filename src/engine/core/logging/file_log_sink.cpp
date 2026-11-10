#include "file_log_sink.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <ios>
#include <stdexcept>
#include <string>
#include <utility>

namespace SNE::Engine::Core::Logging {
    FileLogSink::FileLogSink(std::filesystem::path path,
                             FileRotationPolicy rotation_policy)
        : m_Path(std::move(path)), m_StreamSink(m_File),
          m_RotationPolicy(rotation_policy) {
        if (m_RotationPolicy.max_file_size_bytes == 0) {
            throw std::invalid_argument{
                "FileLogSink: max_file_size_bytes must be greater than zero"};
        }

        m_File.exceptions(std::ios::badbit | std::ios::failbit);

        try {
            m_File.open(m_Path, std::ios::out | std::ios::app);
        } catch (const std::ios_base::failure &error) {
            throw Error::EngineError(
                Error::Code::FileLogOpenFailed, "Failed to open log file",
                Error::NativeError(error.code().value(), error.what()),
                "Open Log File");
        }

        try {
            m_CurrentFileSize = std::filesystem::file_size(m_Path);
        } catch (const std::filesystem::filesystem_error &error) {
            throw Error::EngineError(
                Error::Code::FileLogOpenFailed,
                "Failed to determine initial log file size",
                Error::NativeError(error.code().value(), error.what()),
                "Query Log File Size");
        }
    }

    auto FileLogSink::archivePath(std::size_t index) const
        -> std::filesystem::path {
        if (index == 0U) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition, Error::Subsystem::Core,
                "Log archive index must be greater than zero");
        }

        auto path = m_Path;

        path += ".";
        path += std::to_string(index);

        return path;
    }

    auto FileLogSink::rotate() -> void {
        m_RotationIncomplete = true;

        try {
            m_File.close();
        } catch (const std::ios_base::failure &error) {
            throw Error::EngineError(
                Error::Code::FileLogRotationFailed,
                "Failed to close active log file during rotation",
                Error::NativeError(error.code().value(), error.what()),
                "Rotate Log File");
        }

        if (m_RotationPolicy.max_archives > 0U) {
            const auto oldest_archive =
                archivePath(m_RotationPolicy.max_archives);

            try {
                std::filesystem::remove(oldest_archive);
            } catch (const std::filesystem::filesystem_error &error) {
                throw Error::EngineError(
                    Error::Code::FileLogRotationFailed,
                    "Failed to remove oldest log archive",
                    Error::NativeError(error.code().value(), error.what()),
                    "Rotate Log File");
            }

            for (std::size_t i{m_RotationPolicy.max_archives}; i > 1U; --i) {
                const auto source_path = archivePath(i - 1);
                const auto destination_path = archivePath(i);

                try {
                    if (std::filesystem::exists(source_path)) {
                        std::filesystem::rename(source_path, destination_path);
                    }
                } catch (const std::filesystem::filesystem_error &error) {
                    throw Error::EngineError(
                        Error::Code::FileLogRotationFailed,
                        "Failed to shift log archive",
                        Error::NativeError(error.code().value(), error.what()),
                        "Rotate Log File");
                }
            }

            try {
                std::filesystem::rename(m_Path, archivePath(1U));
            } catch (const std::filesystem::filesystem_error &error) {
                throw Error::EngineError(
                    Error::Code::FileLogRotationFailed,
                    "Failed to archive active log file",
                    Error::NativeError(error.code().value(), error.what()),
                    "Rotate Log File");
            }
        }

        try {
            m_File.open(m_Path, std::ios::out | std::ios::trunc);
        } catch (const std::ios_base::failure &error) {
            throw Error::EngineError(
                Error::Code::FileLogRotationFailed,
                "Failed to reopen active log file after rotation",
                Error::NativeError(error.code().value(), error.what()),
                "Rotate Log File");
        }

        m_CurrentFileSize = 0U;
        m_RotationIncomplete = false;
    }

    auto FileLogSink::write(const LogRecord &record) -> void {
        if (m_RotationIncomplete) {
            throw Error::EngineError(
                Error::Code::FileLogRotationFailed,
                "Cannot write log record after incomplete rotation",
                "Write Log File");
        }

        if (m_CurrentFileSize >= m_RotationPolicy.max_file_size_bytes) {
            rotate();
        }

        try {
            m_StreamSink.write(record);

            const auto position = m_File.tellp();

            if (position == std::streampos{-1}) {
                throw Error::EngineError(
                    Error::Code::FileLogWriteFailed,
                    "Failed to determine log file position", "Write Log File");
            }

            const auto offset = static_cast<std::streamoff>(position);

            if (offset < 0) {
                throw Error::EngineError(
                    Error::Code::FileLogWriteFailed,
                    "Log file position must not be negative", "Write Log File");
            }

            m_CurrentFileSize = static_cast<std::uintmax_t>(offset);
        } catch (const std::ios_base::failure &error) {
            throw Error::EngineError(
                Error::Code::FileLogWriteFailed, "Failed to write log record",
                Error::NativeError(error.code().value(), error.what()),
                "Write Log File");
        }
    }
} // namespace SNE::Engine::Core::Logging
