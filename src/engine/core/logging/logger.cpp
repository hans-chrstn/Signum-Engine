#include "logger.hpp"
#include "engine/core/logging/log_level_filter.hpp"
#include "engine/core/logging/log_record.hpp"
#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace SNE::Engine::Core::Logging {
    auto Logger::addSink(std::unique_ptr<LogSink> sink) -> void {
        if (!sink) {
            throw std::invalid_argument(
                "Logger::addSink: sink must not be null");
        }

        m_Sinks.push_back(std::move(sink));
    }

    auto Logger::setMinimumLevel(LogLevel level) noexcept -> void {
        m_MinimumLevel = level;
    }

    auto Logger::minimumLevel() const noexcept -> LogLevel {
        return m_MinimumLevel;
    }

    // NOLINTBEGIN(bugprone-easily-swappable-parameters)
    auto Logger::log(LogLevel level, std::string_view category,
                     std::string_view message) -> void {
        if (!passesLevelFilter(level, m_MinimumLevel)) {
            return;
        }

        const LogRecord record{
            .level = level,
            .category = std::string{category},
            .message = std::string{message},
            .timestamp = std::chrono::system_clock::now(),
        };

        for (const auto &sink : m_Sinks) {
            sink->write(record);
        }
    }
    // NOLINTEND(bugprone-easily-swappable-parameters)
} // namespace SNE::Engine::Core::Logging
