#include "engine/core/logging/log_level.hpp"

#include <cstdint>
#include <gtest/gtest.h>

namespace Logging = SNE::Engine::Core::Logging;

namespace {
    constexpr std::uint8_t kUnknownLogLevelValue = 255U;
}

TEST(LogLevelTests, KnownLevelsHaveReadableNames) {
    EXPECT_EQ(Logging::toString(Logging::LogLevel::Trace), "Trace");
    EXPECT_EQ(Logging::toString(Logging::LogLevel::Debug), "Debug");
    EXPECT_EQ(Logging::toString(Logging::LogLevel::Info), "Info");
    EXPECT_EQ(Logging::toString(Logging::LogLevel::Warning), "Warning");
    EXPECT_EQ(Logging::toString(Logging::LogLevel::Error), "Error");
    EXPECT_EQ(Logging::toString(Logging::LogLevel::Fatal), "Fatal");
}

TEST(LogLevelTests, UnknownLevelHasFallbackName) {
    const auto unknown_level =
        // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
        static_cast<Logging::LogLevel>(kUnknownLogLevelValue);

    EXPECT_EQ(Logging::toString(unknown_level), "Unknown");
}
