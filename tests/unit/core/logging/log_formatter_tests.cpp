#include "engine/core/logging/log_formatter.hpp"
#include "engine/core/logging/log_level.hpp"
#include "engine/core/logging/log_record.hpp"
#include <chrono>
#include <gtest/gtest.h>
#include <string>

namespace Logging = SNE::Engine::Core::Logging;

namespace {
    using namespace std::chrono_literals;
}

TEST(LogFormatterTests, FormatsAllFieldsIntoSingleLine) {
    const Logging::LogRecord record{
        .level = Logging::LogLevel::Warning,
        .category = "Renderer",
        .message = "Shader reload failed",
        .timestamp = std::chrono::sys_days{std::chrono::year{2026} / 11 / 8} +
                     13h + 29min + 37s,
    };

    EXPECT_EQ(Logging::formatLogRecord(record),
              "[2026-11-08T13:29:37Z] [Warning] [Renderer] "
              "Shader reload failed");
}

TEST(LogFormatterTests, FloorsSubsecondTimestampPrecision) {
    const Logging::LogRecord record{
        .level = Logging::LogLevel::Info,
        .category = "Core",
        .message = "Engine initialized",
        .timestamp = std::chrono::sys_days{std::chrono::year{2026} / 11 / 8} +
                     13h + 29min + 37s + 789ms,
    };

    EXPECT_EQ(Logging::formatLogRecord(record),
              "[2026-11-08T13:29:37Z] [Info] [Core] Engine initialized");
}

TEST(LogFormatterTests, PreservesEmptyCategoryAndMessage) {
    const Logging::LogRecord record{
        .level = Logging::LogLevel::Debug,
        .category = "",
        .message = "",
        .timestamp = std::chrono::sys_days{std::chrono::year{2026} / 11 / 8},
    };

    EXPECT_EQ(Logging::formatLogRecord(record),
              "[2026-11-08T00:00:00Z] [Debug] [] ");
}
