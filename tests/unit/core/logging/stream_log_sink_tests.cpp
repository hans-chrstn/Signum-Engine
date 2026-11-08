#include "engine/core/logging/stream_log_sink.hpp"
#include <chrono>
#include <gtest/gtest.h>
#include <sstream>

namespace Logging = SNE::Engine::Core::Logging;

namespace {
    constexpr auto kTestTimestamp =
        std::chrono::sys_days{std::chrono::year{2026} / 11 / 8} +
        std::chrono::hours{18} + std::chrono::minutes{30} +
        std::chrono::seconds{15};
} // namespace

TEST(StreamLogSinkTests, WritesFormattedRecordWithNewline) {
    std::ostringstream output;
    Logging::StreamLogSink sink{output};

    const Logging::LogRecord record{
        .level = Logging::LogLevel::Warning,
        .category = "Renderer",
        .message = "Shader compilation failed",
        .timestamp = kTestTimestamp,
    };

    sink.write(record);

    EXPECT_EQ(output.str(), "[2026-11-08T18:30:15Z] [Warning] [Renderer] "
                            "Shader compilation failed\n");
}

TEST(StreamLogSinkTests, SupportsPolymorphicDispatch) {
    std::ostringstream output;
    Logging::StreamLogSink concrete_sink{output};

    Logging::LogSink &sink = concrete_sink;

    const Logging::LogRecord record{
        .level = Logging::LogLevel::Info,
        .category = "Core",
        .message = "Engine initialized",
        .timestamp = kTestTimestamp,
    };

    sink.write(record);

    EXPECT_EQ(output.str(), "[2026-11-08T18:30:15Z] [Info] [Core] "
                            "Engine initialized\n");
}

TEST(StreamLogSinkTests, WritesConsecutiveRecordsOnSeparateLines) {
    std::ostringstream output;
    Logging::StreamLogSink sink{output};

    const Logging::LogRecord first{
        .level = Logging::LogLevel::Info,
        .category = "Core",
        .message = "Starting",
        .timestamp = kTestTimestamp,
    };

    const Logging::LogRecord second{
        .level = Logging::LogLevel::Debug,
        .category = "Renderer",
        .message = "Ready",
        .timestamp = kTestTimestamp,
    };

    sink.write(first);
    sink.write(second);

    EXPECT_EQ(output.str(),
              "[2026-11-08T18:30:15Z] [Info] [Core] Starting\n"
              "[2026-11-08T18:30:15Z] [Debug] [Renderer] Ready\n");
}
