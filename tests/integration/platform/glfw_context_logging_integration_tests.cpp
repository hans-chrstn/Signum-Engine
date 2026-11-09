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

        ASSERT_EQ(records.size(), 2U);

        EXPECT_EQ(error.getCode(), Error::Code::GlfwInitializationFailed);

        ASSERT_TRUE(error.getNativeError().has_value());

        EXPECT_NE(error.getNativeError()->getNativeCode(), GLFW_NO_ERROR);
        EXPECT_EQ(records[0].level, Logging::LogLevel::Error);
        EXPECT_EQ(records[0].category, "Platform");

        EXPECT_NE(records[0].message.find("GLFW error "), std::string::npos);

        EXPECT_NE(records[0].message.find(
                      std::to_string(error.getNativeError()->getNativeCode())),
                  std::string::npos);

        EXPECT_NE(records[0].message.find(
                      error.getNativeError()->getNativeDescription()),
                  std::string::npos);

        EXPECT_EQ(records[1].level, Logging::LogLevel::Error);
        EXPECT_EQ(records[1].category, "Platform");

        EXPECT_NE(records[1].message.find("GLFW initialization failed:"),
                  std::string::npos);

        EXPECT_NE(records[1].message.find(
                      error.getNativeError()->getNativeDescription()),
                  std::string::npos);
    }
}

TEST(GlfwContextLoggingIntegrationTests, ReportsRuntimeGlfwErrorsToLogger) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    std::vector<Logging::LogRecord> records;

    Logging::Logger logger{std::make_unique<CapturingSink>(records)};

    Platform::GlfwContext context{&logger};

    ASSERT_EQ(records.size(), 1U);
    EXPECT_EQ(records[0].message, "GLFW initialized");

    ASSERT_EQ(glfwGetCurrentContext(), nullptr);

    glfwSwapInterval(1);

    EXPECT_EQ(glfwGetError(nullptr), GLFW_NO_CURRENT_CONTEXT);

    ASSERT_EQ(records.size(), 2U);

    EXPECT_EQ(records[1].level, Logging::LogLevel::Error);
    EXPECT_EQ(records[1].category, "Platform");

    const std::string expected_prefix =
        "GLFW error " + std::to_string(GLFW_NO_CURRENT_CONTEXT) + ": ";

    EXPECT_TRUE(records[1].message.starts_with(expected_prefix));
}

TEST(GlfwContextLoggingIntegrationTests,
     ReinitializationUsesNewLoggerAndUnregistersCallback) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    std::vector<Logging::LogRecord> first_records;
    std::vector<Logging::LogRecord> second_records;

    Logging::Logger first_logger{
        std::make_unique<CapturingSink>(first_records)};

    Logging::Logger second_logger{
        std::make_unique<CapturingSink>(second_records)};

    {
        Platform::GlfwContext context{&first_logger};

        ASSERT_EQ(first_records.size(), 1U);

        ASSERT_EQ(glfwGetCurrentContext(), nullptr);

        glfwSwapInterval(1);

        EXPECT_EQ(glfwGetError(nullptr), GLFW_NO_CURRENT_CONTEXT);

        ASSERT_EQ(first_records.size(), 2U);
        EXPECT_EQ(first_records[1].level, Logging::LogLevel::Error);
    }

    EXPECT_EQ(glfwSetErrorCallback(nullptr), nullptr);

    const std::size_t previous_count = first_records.size();

    {
        Platform::GlfwContext context{&second_logger};

        ASSERT_EQ(second_records.size(), 1U);
        EXPECT_EQ(second_records[0].message, "GLFW initialized");

        ASSERT_EQ(glfwGetCurrentContext(), nullptr);

        glfwSwapInterval(1);

        EXPECT_EQ(glfwGetError(nullptr), GLFW_NO_CURRENT_CONTEXT);

        ASSERT_EQ(second_records.size(), 2U);

        EXPECT_EQ(second_records[1].level, Logging::LogLevel::Error);
        EXPECT_EQ(second_records[1].category, "Platform");

        EXPECT_EQ(first_records.size(), previous_count);
    }

    EXPECT_EQ(glfwSetErrorCallback(nullptr), nullptr);
    EXPECT_EQ(first_records.size(), previous_count);
}
