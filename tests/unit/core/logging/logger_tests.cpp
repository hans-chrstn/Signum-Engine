#include "engine/core/logging/log_categories.hpp"
#include "engine/core/logging/logger.hpp"
#include "engine/core/logging/stream_log_sink.hpp"
#include <barrier>
#include <chrono>
#include <gtest/gtest.h>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_set>
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

    class ReentrantLoggerSink final : public Logging::LogSink {
      private:
        Logging::Logger &m_Logger;
        bool m_HasLoggedNested{false};

      public:
        explicit ReentrantLoggerSink(Logging::Logger &logger)
            : m_Logger(logger) {}

        auto write(const Logging::LogRecord &record) -> void override {
            if (record.message == "Outer event" && !m_HasLoggedNested) {
                m_HasLoggedNested = true;

                m_Logger.log(Logging::LogLevel::Info, Logging::Categories::Core,
                             "Nested event");
            }
        }
    };

    class ThrowOnceLoggerSink final : public Logging::LogSink {
      private:
        bool m_ShouldThrow{true};

      public:
        auto write([[maybe_unused]] const Logging::LogRecord &record)
            -> void override {
            if (m_ShouldThrow) {
                m_ShouldThrow = false;
                throw std::runtime_error{"Simulated sink failure"};
            }
        }
    };

    class EnqueueThenThrowOnceSink final : public Logging::LogSink {
      private:
        Logging::Logger &m_Logger;
        bool m_ShouldThrow{true};

      public:
        explicit EnqueueThenThrowOnceSink(Logging::Logger &logger)
            : m_Logger(logger) {}

        auto write([[maybe_unused]] const Logging::LogRecord &record)
            -> void override {
            if (m_ShouldThrow) {
                m_ShouldThrow = false;

                m_Logger.log(Logging::LogLevel::Info, Logging::Categories::Core,
                             "Pending event");

                throw std::runtime_error{"Simulated dispatch failure"};
            }
        }
    };

    class RegisteringLoggerSink final : public Logging::LogSink {
      private:
        Logging::Logger &m_Logger;
        std::vector<Logging::LogRecord> &m_NewSinkRecords;
        bool m_HasRegistered{false};

      public:
        RegisteringLoggerSink(Logging::Logger &logger,
                              std::vector<Logging::LogRecord> &new_sink_records)
            : m_Logger(logger), m_NewSinkRecords(new_sink_records) {}

        auto write([[maybe_unused]] const Logging::LogRecord &record)
            -> void override {
            if (!m_HasRegistered) {
                m_HasRegistered = true;

                m_Logger.addSink(
                    std::make_unique<CapturingLoggerSink>(m_NewSinkRecords));
            }
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
    std::vector<Logging::LogRecord> records;
    Logging::Logger logger;

    logger.addSink(std::make_unique<CapturingLoggerSink>(records));

    const auto before = std::chrono::system_clock::now();

    logger.log(Logging::LogLevel::Warning, Logging::Categories::Renderer,
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
    std::vector<Logging::LogRecord> records;
    Logging::Logger logger;

    logger.addSink(std::make_unique<CapturingLoggerSink>(records));
    logger.setMinimumLevel(Logging::LogLevel::Warning);

    logger.log(Logging::LogLevel::Trace, Logging::Categories::Core,
               "Trace message");
    logger.log(Logging::LogLevel::Debug, Logging::Categories::Core,
               "Debug message");
    logger.log(Logging::LogLevel::Info, Logging::Categories::Core,
               "Info message");

    EXPECT_TRUE(records.empty());

    logger.log(Logging::LogLevel::Warning, Logging::Categories::Core,
               "Warning message");

    ASSERT_EQ(records.size(), 1U);
    EXPECT_EQ(records[0].message, "Warning message");
}

TEST(LoggerTests, DispatchesSameEventToMultipleSinks) {
    std::vector<Logging::LogRecord> first_records;
    std::vector<Logging::LogRecord> second_records;
    Logging::Logger logger;

    logger.addSink(std::make_unique<CapturingLoggerSink>(first_records));

    logger.addSink(std::make_unique<CapturingLoggerSink>(second_records));

    logger.log(Logging::LogLevel::Error, Logging::Categories::Vulkan,
               "Device operation failed");

    ASSERT_EQ(first_records.size(), 1U);
    ASSERT_EQ(second_records.size(), 1U);

    EXPECT_EQ(first_records[0].message, second_records[0].message);
    EXPECT_EQ(first_records[0].category, second_records[0].category);
    EXPECT_EQ(first_records[0].level, second_records[0].level);
    EXPECT_EQ(first_records[0].timestamp, second_records[0].timestamp);
}

TEST(LoggerTests, AllowsOnlyRegisteredCategories) {
    std::vector<Logging::LogRecord> records;
    Logging::Logger logger;

    logger.addSink(std::make_unique<CapturingLoggerSink>(records));

    logger.allowCategory(Logging::Categories::Renderer);
    logger.allowCategory(Logging::Categories::Vulkan);

    logger.log(Logging::LogLevel::Info, Logging::Categories::Renderer,
               "Mesh uploaded");
    logger.log(Logging::LogLevel::Info, Logging::Categories::Editor,
               "Panel opened");
    logger.log(Logging::LogLevel::Info, Logging::Categories::Vulkan,
               "Device initialized");

    ASSERT_EQ(records.size(), 2U);

    EXPECT_EQ(records[0].category, "Renderer");
    EXPECT_EQ(records[1].category, "Vulkan");
}

TEST(LoggerTests, CategoryAllowlistIsCaseSensitive) {
    std::vector<Logging::LogRecord> records;
    Logging::Logger logger;

    logger.addSink(std::make_unique<CapturingLoggerSink>(records));
    logger.allowCategory(Logging::Categories::Renderer);

    logger.log(Logging::LogLevel::Info, "renderer", "Lowercase");
    logger.log(Logging::LogLevel::Info, "RENDERER", "Uppercase");
    logger.log(Logging::LogLevel::Info, Logging::Categories::Renderer,
               "Exact match");

    ASSERT_EQ(records.size(), 1U);
    EXPECT_EQ(records[0].message, "Exact match");
}

TEST(LoggerTests, CategoryRegistrationOwnsSourceString) {
    std::vector<Logging::LogRecord> records;
    Logging::Logger logger;

    logger.addSink(std::make_unique<CapturingLoggerSink>(records));

    std::string category = "Renderer";
    logger.allowCategory(category);

    category = "Editor";

    logger.log(Logging::LogLevel::Info, Logging::Categories::Renderer,
               "Original category");
    logger.log(Logging::LogLevel::Info, Logging::Categories::Editor,
               "Changed category");

    ASSERT_EQ(records.size(), 1U);
    EXPECT_EQ(records[0].category, "Renderer");
    EXPECT_EQ(records[0].message, "Original category");
}

TEST(LoggerTests, RepeatedCategoryRegistrationPreservesFiltering) {
    std::vector<Logging::LogRecord> records;
    Logging::Logger logger;

    logger.addSink(std::make_unique<CapturingLoggerSink>(records));

    logger.allowCategory(Logging::Categories::Renderer);
    logger.allowCategory(Logging::Categories::Renderer);
    logger.allowCategory(Logging::Categories::Renderer);

    logger.log(Logging::LogLevel::Info, Logging::Categories::Renderer,
               "Accepted");
    logger.log(Logging::LogLevel::Info, Logging::Categories::Editor,
               "Rejected");

    ASSERT_EQ(records.size(), 1U);
    EXPECT_EQ(records[0].message, "Accepted");
}

TEST(LoggerTests, AppliesSeverityAndCategoryFiltersTogether) {
    std::vector<Logging::LogRecord> records;
    Logging::Logger logger;

    logger.addSink(std::make_unique<CapturingLoggerSink>(records));

    logger.setMinimumLevel(Logging::LogLevel::Warning);
    logger.allowCategory(Logging::Categories::Renderer);

    logger.log(Logging::LogLevel::Info, Logging::Categories::Renderer,
               "Below threshold");
    logger.log(Logging::LogLevel::Error, Logging::Categories::Editor,
               "Wrong category");
    logger.log(Logging::LogLevel::Error, Logging::Categories::Renderer,
               "Accepted");

    ASSERT_EQ(records.size(), 1U);
    EXPECT_EQ(records[0].message, "Accepted");
}

TEST(LoggerTests, ClearingAllowedCategoriesRestoresAllCategories) {
    std::vector<Logging::LogRecord> records;
    Logging::Logger logger;

    logger.addSink(std::make_unique<CapturingLoggerSink>(records));

    logger.allowCategory(Logging::Categories::Renderer);

    logger.log(Logging::LogLevel::Info, Logging::Categories::Editor,
               "Before clearing");

    EXPECT_TRUE(records.empty());

    logger.clearAllowedCategories();

    logger.log(Logging::LogLevel::Info, Logging::Categories::Editor,
               "After clearing");
    logger.log(Logging::LogLevel::Info, Logging::Categories::Vulkan,
               "Device initialized");

    ASSERT_EQ(records.size(), 2U);
    EXPECT_EQ(records[0].message, "After clearing");
    EXPECT_EQ(records[1].category, "Vulkan");
}

TEST(LoggerTests, ClearingCategoriesPreservesSeverityFiltering) {
    std::vector<Logging::LogRecord> records;
    Logging::Logger logger;

    logger.addSink(std::make_unique<CapturingLoggerSink>(records));

    logger.setMinimumLevel(Logging::LogLevel::Warning);
    logger.allowCategory(Logging::Categories::Renderer);

    logger.clearAllowedCategories();

    logger.log(Logging::LogLevel::Info, Logging::Categories::Editor,
               "Below severity");
    logger.log(Logging::LogLevel::Error, Logging::Categories::Editor,
               "Accepted");

    ASSERT_EQ(records.size(), 1U);
    EXPECT_EQ(records[0].message, "Accepted");
}

TEST(LoggerTests, ReentrantLoggingQueuesNestedRecord) {
    std::vector<Logging::LogRecord> records;
    Logging::Logger logger;

    logger.addSink(std::make_unique<CapturingLoggerSink>(records));
    logger.addSink(std::make_unique<ReentrantLoggerSink>(logger));

    logger.log(Logging::LogLevel::Info, Logging::Categories::Core,
               "Outer event");

    ASSERT_EQ(records.size(), 2U);
    EXPECT_EQ(records[0].message, "Outer event");
    EXPECT_EQ(records[1].message, "Nested event");
}

TEST(LoggerTests, ConcurrentProducersDeliverEveryRecordExactlyOnce) {
    constexpr int kThreadCount = 4;
    constexpr int kRecordsPerThread = 100;

    std::vector<Logging::LogRecord> records;
    Logging::Logger logger;

    logger.addSink(std::make_unique<CapturingLoggerSink>(records));

    std::vector<std::jthread> workers;
    workers.reserve(kThreadCount);

    for (int thread_index = 0; thread_index < kThreadCount; ++thread_index) {
        workers.emplace_back([&logger, thread_index] -> void {
            for (int record_index = 0; record_index < kRecordsPerThread;
                 ++record_index) {
                const std::string message = std::to_string(thread_index) + ":" +
                                            std::to_string(record_index);

                logger.log(Logging::LogLevel::Info, "Concurrency", message);
            }
        });
    }

    for (auto &worker : workers) {
        worker.join();
    }

    ASSERT_EQ(records.size(),
              static_cast<std::size_t>(kThreadCount * kRecordsPerThread));

    std::unordered_set<std::string> unique_messages;

    for (const auto &record : records) {
        EXPECT_EQ(record.category, "Concurrency");
        EXPECT_TRUE(unique_messages.insert(record.message).second);
    }

    EXPECT_EQ(unique_messages.size(), records.size());
}

TEST(LoggerTests, SinkFailureDoesNotPermanentlyDisableLogging) {
    std::vector<Logging::LogRecord> records;
    Logging::Logger logger;

    logger.addSink(std::make_unique<ThrowOnceLoggerSink>());
    logger.addSink(std::make_unique<CapturingLoggerSink>(records));

    EXPECT_THROW(logger.log(Logging::LogLevel::Info, Logging::Categories::Core,
                            "First event"),
                 std::runtime_error);

    EXPECT_NO_THROW(logger.log(Logging::LogLevel::Info,
                               Logging::Categories::Core, "Second event"));

    ASSERT_EQ(records.size(), 1U);
    EXPECT_EQ(records[0].message, "Second event");
}

TEST(LoggerTests, PendingRecordsSurviveSinkFailure) {
    std::vector<Logging::LogRecord> records;
    Logging::Logger logger;

    logger.addSink(std::make_unique<EnqueueThenThrowOnceSink>(logger));
    logger.addSink(std::make_unique<CapturingLoggerSink>(records));

    EXPECT_THROW(logger.log(Logging::LogLevel::Info, Logging::Categories::Core,
                            "First event"),
                 std::runtime_error);

    EXPECT_TRUE(records.empty());

    EXPECT_NO_THROW(logger.log(Logging::LogLevel::Info,
                               Logging::Categories::Core, "Recovery event"));

    ASSERT_EQ(records.size(), 2U);
    EXPECT_EQ(records[0].message, "Pending event");
    EXPECT_EQ(records[1].message, "Recovery event");
}

TEST(LoggerTests, SinkRegistrationDuringDispatchAffectsSubsequentRecords) {
    std::vector<Logging::LogRecord> records;
    Logging::Logger logger;

    logger.addSink(std::make_unique<RegisteringLoggerSink>(logger, records));

    logger.log(Logging::LogLevel::Info, Logging::Categories::Core,
               "First event");

    EXPECT_TRUE(records.empty());

    logger.log(Logging::LogLevel::Info, Logging::Categories::Core,
               "Second event");

    ASSERT_EQ(records.size(), 1U);
    EXPECT_EQ(records[0].message, "Second event");
}

TEST(LoggerTests, ConcurrentConfigurationChangesPreserveAcceptedRecords) {
    constexpr int kRecordCount = 500;

    std::vector<Logging::LogRecord> records;
    Logging::Logger logger;

    logger.addSink(std::make_unique<CapturingLoggerSink>(records));

    std::barrier start{3};

    {
        std::jthread severity_worker{[&] -> void {
            start.arrive_and_wait();

            for (int i = 0; i < kRecordCount; ++i) {
                logger.setMinimumLevel(i % 2 == 0 ? Logging::LogLevel::Info
                                                  : Logging::LogLevel::Warning);

                (void)logger.minimumLevel();
            }
        }};

        std::jthread category_worker{[&] -> void {
            start.arrive_and_wait();

            for (int i = 0; i < kRecordCount; ++i) {
                logger.allowCategory(Logging::Categories::Renderer);
                logger.clearAllowedCategories();
            }
        }};

        std::jthread producer{[&] -> void {
            start.arrive_and_wait();

            for (int i = 0; i < kRecordCount; ++i) {
                logger.log(Logging::LogLevel::Warning,
                           Logging::Categories::Renderer, std::to_string(i));
            }
        }};
    }

    ASSERT_EQ(records.size(), static_cast<std::size_t>(kRecordCount));

    std::unordered_set<std::string> unique_messages;

    for (const auto &record : records) {
        EXPECT_EQ(record.category, "Renderer");
        EXPECT_TRUE(unique_messages.insert(record.message).second);
    }

    EXPECT_EQ(unique_messages.size(), records.size());
}

TEST(LoggerTests, DispatchesToStreamSinkThroughLogger) {
    std::ostringstream output;
    Logging::Logger logger;

    logger.addSink(std::make_unique<Logging::StreamLogSink>(output));

    logger.log(Logging::LogLevel::Info, Logging::Categories::Core,
               "Engine initialized");

    const std::string formatted_output = output.str();

    EXPECT_NE(formatted_output.find("[Info] [Core] Engine initialized\n"),
              std::string::npos);
}

TEST(LoggerTests, ConstructorRejectsNullInitialSink) {
    EXPECT_THROW(Logging::Logger(std::unique_ptr<Logging::LogSink>{}),
                 std::invalid_argument);
}

TEST(LoggerTests, ConstructorRegistersInitialSink) {
    std::vector<Logging::LogRecord> records;

    Logging::Logger logger{std::make_unique<CapturingLoggerSink>(records)};

    logger.log(Logging::LogLevel::Info, Logging::Categories::Core,
               "Logger initialized");

    ASSERT_EQ(records.size(), 1U);
    EXPECT_EQ(records[0].level, Logging::LogLevel::Info);
    EXPECT_EQ(records[0].category, "Core");
    EXPECT_EQ(records[0].message, "Logger initialized");
}

TEST(LoggerTests, ConstructorTakesOwnershipOfInitialSink) {
    int destruction_count = 0;

    {
        Logging::Logger logger{
            std::make_unique<DestructionTrackingSink>(destruction_count)};

        EXPECT_EQ(destruction_count, 0);
    }

    EXPECT_EQ(destruction_count, 1);
}

TEST(LoggerTests, MultipleSinkConstructorAcceptsEmptyCollection) {
    std::vector<std::unique_ptr<Logging::LogSink>> sinks;

    Logging::Logger logger{std::move(sinks), Logging::LogLevel::Warning};

    EXPECT_EQ(logger.minimumLevel(), Logging::LogLevel::Warning);

    EXPECT_NO_THROW(logger.log(Logging::LogLevel::Warning,
                               Logging::Categories::Core, "No destinations"));
}

TEST(LoggerTests, MultipleSinkConstructorDispatchesToAllSinks) {
    std::vector<Logging::LogRecord> first_records;
    std::vector<Logging::LogRecord> second_records;

    std::vector<std::unique_ptr<Logging::LogSink>> sinks;

    sinks.push_back(std::make_unique<CapturingLoggerSink>(first_records));

    sinks.push_back(std::make_unique<CapturingLoggerSink>(second_records));

    Logging::Logger logger{std::move(sinks), Logging::LogLevel::Info};

    logger.log(Logging::LogLevel::Info, Logging::Categories::Core,
               "Engine initialized");

    ASSERT_EQ(first_records.size(), 1U);
    ASSERT_EQ(second_records.size(), 1U);

    EXPECT_EQ(first_records[0].message, "Engine initialized");
    EXPECT_EQ(second_records[0].message, "Engine initialized");

    EXPECT_EQ(first_records[0].category, second_records[0].category);
    EXPECT_EQ(first_records[0].level, second_records[0].level);
    EXPECT_EQ(first_records[0].timestamp, second_records[0].timestamp);
}

TEST(LoggerTests, MultipleSinkConstructorAppliesInitialSeverity) {
    std::vector<Logging::LogRecord> records;

    std::vector<std::unique_ptr<Logging::LogSink>> sinks;

    sinks.push_back(std::make_unique<CapturingLoggerSink>(records));

    Logging::Logger logger{std::move(sinks), Logging::LogLevel::Error};

    EXPECT_EQ(logger.minimumLevel(), Logging::LogLevel::Error);

    logger.log(Logging::LogLevel::Info, Logging::Categories::Core, "Ignored");
    logger.log(Logging::LogLevel::Warning, Logging::Categories::Core,
               "Ignored");
    logger.log(Logging::LogLevel::Error, Logging::Categories::Core, "Accepted");

    ASSERT_EQ(records.size(), 1U);

    EXPECT_EQ(records[0].level, Logging::LogLevel::Error);
    EXPECT_EQ(records[0].message, "Accepted");
}

TEST(LoggerTests, MultipleSinkConstructorRejectsNullSink) {
    int destruction_count = 0;

    std::vector<std::unique_ptr<Logging::LogSink>> sinks;

    sinks.push_back(
        std::make_unique<DestructionTrackingSink>(destruction_count));

    sinks.push_back(nullptr);

    sinks.push_back(
        std::make_unique<DestructionTrackingSink>(destruction_count));

    const auto construct_logger = [&sinks] -> void {
        Logging::Logger logger{std::move(sinks), Logging::LogLevel::Info};
    };

    EXPECT_THROW(construct_logger(), std::invalid_argument);

    EXPECT_EQ(destruction_count, 2);
}

TEST(LoggerTests, MultipleSinkConstructorOwnsAllSinks) {
    int destruction_count = 0;

    {
        std::vector<std::unique_ptr<Logging::LogSink>> sinks;

        sinks.push_back(
            std::make_unique<DestructionTrackingSink>(destruction_count));

        sinks.push_back(
            std::make_unique<DestructionTrackingSink>(destruction_count));

        Logging::Logger logger{std::move(sinks), Logging::LogLevel::Info};

        EXPECT_EQ(destruction_count, 0);
    }

    EXPECT_EQ(destruction_count, 2);
}
