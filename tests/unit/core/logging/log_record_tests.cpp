#include "engine/core/logging/log_record.hpp"

#include <chrono>
#include <gtest/gtest.h>

namespace Logging = SNE::Engine::Core::Logging;

namespace {
    constexpr auto kTestTimestamp =
        std::chrono::system_clock::time_point{std::chrono::seconds{12345}};
}

TEST(LogRecordTests, PreservesEventData) {
    const Logging::LogRecord record{
        .level = Logging::LogLevel::Warning,
        .category = "Renderer.ShaderCompilation",
        .message = "Shader compilation failed",
        .timestamp = kTestTimestamp,
    };

    EXPECT_EQ(record.level, Logging::LogLevel::Warning);
    EXPECT_EQ(record.category, "Renderer.ShaderCompilation");
    EXPECT_EQ(record.message, "Shader compilation failed");
    EXPECT_EQ(record.timestamp, kTestTimestamp);
}
