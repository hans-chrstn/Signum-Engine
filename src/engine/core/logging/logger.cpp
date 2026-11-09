#include "logger.hpp"
#include "engine/core/logging/log_category_filter.hpp"
#include "engine/core/logging/log_level_filter.hpp"
#include "engine/core/logging/log_record.hpp"
#include "engine/core/logging/log_sink.hpp"
#include <algorithm>
#include <chrono>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace SNE::Engine::Core::Logging {
    Logger::Logger(std::unique_ptr<LogSink> initial_sink) {
        addSink(std::move(initial_sink));
    }

    auto Logger::drainPendingRecords() -> void {
        try {
            while (true) {
                std::optional<LogRecord> record;
                std::vector<LogSink *> sinks;

                {
                    std::scoped_lock lock{m_Mutex};

                    if (m_PendingRecords.empty()) {
                        m_IsDraining = false;
                        return;
                    }

                    sinks.reserve(m_Sinks.size());

                    for (const auto &sink : m_Sinks) {
                        sinks.push_back(sink.get());
                    }

                    record.emplace(std::move(m_PendingRecords.front()));
                    m_PendingRecords.pop();
                }

                for (const auto &sink : sinks) {
                    sink->write(*record);
                }
            }
        } catch (...) {
            {
                std::scoped_lock lock{m_Mutex};
                m_IsDraining = false;
            }

            throw;
        }
    }

    auto Logger::addSink(std::unique_ptr<LogSink> sink) -> void {
        if (!sink) {
            throw std::invalid_argument(
                "Logger::addSink: sink must not be null");
        }

        std::scoped_lock<std::mutex> lock{m_Mutex};

        m_Sinks.push_back(std::move(sink));
    }

    auto Logger::setMinimumLevel(LogLevel level) -> void {
        std::scoped_lock<std::mutex> lock{m_Mutex};
        m_MinimumLevel = level;
    }

    auto Logger::minimumLevel() const -> LogLevel {
        std::scoped_lock<std::mutex> lock{m_Mutex};
        return m_MinimumLevel;
    }

    // NOLINTBEGIN(bugprone-easily-swappable-parameters)
    auto Logger::log(LogLevel level, std::string_view category,
                     std::string_view message) -> void {
        {
            std::scoped_lock<std::mutex> lock{m_Mutex};
            if (!passesLevelFilter(level, m_MinimumLevel)) {
                return;
            }

            if (!passesCategoryFilter(category, m_AllowedCategories)) {
                return;
            }
        }

        LogRecord record{
            .level = level,
            .category = std::string{category},
            .message = std::string{message},
            .timestamp = std::chrono::system_clock::now(),
        };

        bool should_drain{false};

        {
            std::scoped_lock<std::mutex> lock{m_Mutex};

            m_PendingRecords.push(std::move(record));

            if (!m_IsDraining) {
                m_IsDraining = true;
                should_drain = true;
            }
        }

        if (should_drain) {
            drainPendingRecords();
        }
    }
    // NOLINTEND(bugprone-easily-swappable-parameters)

    auto Logger::allowCategory(std::string_view category) -> void {
        std::scoped_lock<std::mutex> lock{m_Mutex};

        const auto result = std::ranges::find(m_AllowedCategories, category);

        if (result != m_AllowedCategories.end()) {
            return;
        }

        m_AllowedCategories.emplace_back(category);
    }

    auto Logger::clearAllowedCategories() -> void {
        std::scoped_lock<std::mutex> lock{m_Mutex};

        m_AllowedCategories.clear();
    }
} // namespace SNE::Engine::Core::Logging
