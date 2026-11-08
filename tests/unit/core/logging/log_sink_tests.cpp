#include "engine/core/logging/log_sink.hpp"
#include <chrono>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <type_traits>

namespace Logging = SNE::Engine::Core::Logging;

namespace {
    class RecordingSink final : public Logging::LogSink {
      private:
        bool &m_Destroyed;
        std::string &m_CapturedMessage;

      public:
        RecordingSink(bool &destroyed, std::string &captured_message)
            : m_Destroyed(destroyed), m_CapturedMessage(captured_message) {}

        ~RecordingSink() override {
            m_Destroyed = true;
        }

        auto write(const Logging::LogRecord &record) -> void override {
            m_CapturedMessage = record.message;
        }
    };

    static_assert(std::is_abstract_v<Logging::LogSink>);
    static_assert(std::has_virtual_destructor_v<Logging::LogSink>);
} // namespace

TEST(LogSinkTests, DispatchesAndDestroysThroughBaseInterface) {
    bool destroyed = false;
    std::string captured_message;

    {
        std::unique_ptr<Logging::LogSink> sink =
            std::make_unique<RecordingSink>(destroyed, captured_message);

        const Logging::LogRecord record{
            .level = Logging::LogLevel::Warning,
            .category = "Renderer",
            .message = "Test diagnostic",
            .timestamp = std::chrono::system_clock::time_point{},
        };

        sink->write(record);

        EXPECT_EQ(captured_message, "Test diagnostic");
        EXPECT_FALSE(destroyed);
    }

    EXPECT_TRUE(destroyed);
}
