#include "engine/core/logging/logger.hpp"
#include <chrono>
#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace Logging = SNE::Engine::Core::Logging;

namespace {
    class DestructionTrackingSink final : public Logging::LogSink {
      private:
        int &m_DestructionCount;

      public:
        explicit DestructionTrackingSink(int &destruction_count)
            : m_DestructionCount(destruction_count) {}

        ~DestructionTrackingSink() override {
            ++m_DestructionCount;
        }

        auto write([[maybe_unused]] const Logging::LogRecord &record)
            -> void override {}
    };

    class CapturingLoggerSink final : public Logging::LogSink {
      private:
        std::vector<Logging::LogRecord> &m_Records;

      public:
        explicit CapturingLoggerSink(std::vector<Logging::LogRecord> &records)
            : m_Records(records) {}

        auto write(const Logging::LogRecord &record) -> void override {
            m_Records.push_back(record);
        }
    };
} // namespace

TEST(LoggerTests, RejectsNullSink) {
    Logging::Logger logger;

    EXPECT_THROW(logger.addSink(std::unique_ptr<Logging::LogSink>{}),
                 std::invalid_argument);
}

TEST(LoggerTests, TakesOwnershipOfRegisteredSink) {
    int destruction_count = 0;

    {
        Logging::Logger logger;

        std::unique_ptr<Logging::LogSink> sink =
            std::make_unique<DestructionTrackingSink>(destruction_count);

        logger.addSink(std::move(sink));

        EXPECT_FALSE(sink);
        EXPECT_EQ(destruction_count, 0);
    }

    EXPECT_EQ(destruction_count, 1);
}

TEST(LoggerTests, DestroysAllRegisteredSinks) {
    int destruction_count = 0;

    {
        Logging::Logger logger;

        logger.addSink(
            std::make_unique<DestructionTrackingSink>(destruction_count));

        logger.addSink(
            std::make_unique<DestructionTrackingSink>(destruction_count));

        EXPECT_EQ(destruction_count, 0);
    }

    EXPECT_EQ(destruction_count, 2);
}

TEST(LoggerTests, DefaultsToInfoMinimumLevel) {
    const Logging::Logger logger;

    EXPECT_EQ(logger.minimumLevel(), Logging::LogLevel::Info);
}

TEST(LoggerTests, UpdatesMinimumLevel) {
    Logging::Logger logger;

    logger.setMinimumLevel(Logging::LogLevel::Warning);
    EXPECT_EQ(logger.minimumLevel(), Logging::LogLevel::Warning);

    logger.setMinimumLevel(Logging::LogLevel::Trace);
    EXPECT_EQ(logger.minimumLevel(), Logging::LogLevel::Trace);

    logger.setMinimumLevel(Logging::LogLevel::Fatal);
    EXPECT_EQ(logger.minimumLevel(), Logging::LogLevel::Fatal);
}

TEST(LoggerTests, DispatchesAcceptedRecordToRegisteredSink) {
    Logging::Logger logger;
    std::vector<Logging::LogRecord> records;

    logger.addSink(std::make_unique<CapturingLoggerSink>(records));

    const auto before = std::chrono::system_clock::now();

    logger.log(Logging::LogLevel::Warning, "Renderer",
               "Pipeline creation failed");

    const auto after = std::chrono::system_clock::now();

    ASSERT_EQ(records.size(), 1U);

    EXPECT_EQ(records[0].level, Logging::LogLevel::Warning);
    EXPECT_EQ(records[0].category, "Renderer");
    EXPECT_EQ(records[0].message, "Pipeline creation failed");
    EXPECT_GE(records[0].timestamp, before);
    EXPECT_LE(records[0].timestamp, after);
}

TEST(LoggerTests, RejectsEventsBelowMinimumLevel) {
    Logging::Logger logger;
    std::vector<Logging::LogRecord> records;

    logger.addSink(std::make_unique<CapturingLoggerSink>(records));
    logger.setMinimumLevel(Logging::LogLevel::Warning);

    logger.log(Logging::LogLevel::Trace, "Core", "Trace message");
    logger.log(Logging::LogLevel::Debug, "Core", "Debug message");
    logger.log(Logging::LogLevel::Info, "Core", "Info message");

    EXPECT_TRUE(records.empty());

    logger.log(Logging::LogLevel::Warning, "Core", "Warning message");

    ASSERT_EQ(records.size(), 1U);
    EXPECT_EQ(records[0].message, "Warning message");
}

TEST(LoggerTests, DispatchesSameEventToMultipleSinks) {
    Logging::Logger logger;

    std::vector<Logging::LogRecord> first_records;
    std::vector<Logging::LogRecord> second_records;

    logger.addSink(std::make_unique<CapturingLoggerSink>(first_records));

    logger.addSink(std::make_unique<CapturingLoggerSink>(second_records));

    logger.log(Logging::LogLevel::Error, "Vulkan", "Device operation failed");

    ASSERT_EQ(first_records.size(), 1U);
    ASSERT_EQ(second_records.size(), 1U);

    EXPECT_EQ(first_records[0].message, second_records[0].message);
    EXPECT_EQ(first_records[0].category, second_records[0].category);
    EXPECT_EQ(first_records[0].level, second_records[0].level);
    EXPECT_EQ(first_records[0].timestamp, second_records[0].timestamp);
}
