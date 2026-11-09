#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/logging/log_level.hpp"
#include "engine/core/logging/log_record.hpp"
#include "engine/core/logging/log_sink.hpp"
#include "engine/core/logging/logger.hpp"
#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace Logging = SNE::Engine::Core::Logging;
namespace Platform = SNE::Engine::Platform;

namespace {
    struct RestoreGlfwPlatformHint {
        ~RestoreGlfwPlatformHint() {
            glfwInitHint(GLFW_PLATFORM, GLFW_ANY_PLATFORM);
        }
    };

    class CapturingSink final : public Logging::LogSink {
      private:
        std::vector<Logging::LogRecord> &m_Records;

      public:
        explicit CapturingSink(std::vector<Logging::LogRecord> &records)
            : m_Records(records) {}

        auto write(const Logging::LogRecord &record) -> void override {
            m_Records.push_back(record);
        }
    };

    class ThrowingSink final : public Logging::LogSink {
      public:
        auto write([[maybe_unused]] const Logging::LogRecord &record)
            -> void override {
            throw std::runtime_error{"Injected logging failure"};
        }
    };

    [[nodiscard]] auto hasDisplay() -> bool {
        return std::getenv("DISPLAY") != nullptr ||
               std::getenv("WAYLAND_DISPLAY") != nullptr;
    }
} // namespace

TEST(GlfwContextLoggingIntegrationTests, EmitsInitializationDiagnostic) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    std::vector<Logging::LogRecord> records;

    Logging::Logger logger{std::make_unique<CapturingSink>(records)};

    {
        Platform::GlfwContext context{&logger};

        ASSERT_EQ(records.size(), 1U);
        EXPECT_EQ(records[0].level, Logging::LogLevel::Info);
        EXPECT_EQ(records[0].category, "Platform");
        EXPECT_EQ(records[0].message, "GLFW initialized");
    }
}

TEST(GlfwContextLoggingIntegrationTests,
     LoggingFailureDoesNotPreventWindowCreation) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Logging::Logger logger{std::make_unique<ThrowingSink>()};

    Platform::GlfwContext context{&logger};

    constexpr int kWindowWidth = 320;
    constexpr int kWindowHeight = 240;

    Platform::Window window{
        {.width = kWindowWidth, .height = kWindowHeight},
        "GLFW Logging Failure Integration Test",
    };

    EXPECT_NE(window.nativeHandle(), nullptr);
}

TEST(GlfwContextLoggingIntegrationTests,
     InitializationFailureLogsAndPreservesEngineError) {
    [[maybe_unused]] RestoreGlfwPlatformHint restore_hint;

    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_COCOA);

    std::vector<Logging::LogRecord> records;

    Logging::Logger logger{std::make_unique<CapturingSink>(records)};

    try {
        Platform::GlfwContext context{&logger};

        FAIL() << "Expected GLFW initialization to fail";
    } catch (const SNE::Engine::Core::Error::EngineError &error) {
        namespace Error = SNE::Engine::Core::Error;

        EXPECT_EQ(error.getCode(), Error::Code::GlfwInitializationFailed);

        ASSERT_TRUE(error.getNativeError().has_value());

        EXPECT_NE(error.getNativeError()->getNativeCode(), GLFW_NO_ERROR);

        ASSERT_EQ(records.size(), 1U);

        EXPECT_EQ(records[0].level, Logging::LogLevel::Error);
        EXPECT_EQ(records[0].category, "Platform");

        EXPECT_NE(records[0].message.find("GLFW initialization failed:"),
                  std::string::npos);

        EXPECT_NE(records[0].message.find(
                      error.getNativeError()->getNativeDescription()),
                  std::string::npos);
    }
}
