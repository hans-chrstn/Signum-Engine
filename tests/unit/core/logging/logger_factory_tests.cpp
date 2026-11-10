#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/logging/logger_factory.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <system_error>

namespace Logging = SNE::Engine::Core::Logging;
namespace Error = SNE::Engine::Core::Error;

namespace {
    class ScopedClogCapture {
      private:
        std::ostringstream m_Output;
        std::streambuf *m_Previous;

      public:
        ScopedClogCapture() : m_Previous(std::clog.rdbuf(m_Output.rdbuf())) {}

        ScopedClogCapture(const ScopedClogCapture &) = delete;
        auto operator=(const ScopedClogCapture &)
            -> ScopedClogCapture & = delete;

        ~ScopedClogCapture() {
            std::clog.rdbuf(m_Previous);
        }

        [[nodiscard]] auto text() const -> std::string {
            return m_Output.str();
        }
    };

    [[nodiscard]] auto readFile(const std::filesystem::path &path)
        -> std::string {
        std::ifstream input{path, std::ios::binary};
        std::ostringstream output;
        output << input.rdbuf();
        return output.str();
    }

    class LoggerFactoryTests : public testing::Test {
      protected:
        std::filesystem::path m_Directory;
        std::filesystem::path m_Path;

        auto SetUp() -> void override {
            const auto timestamp =
                std::chrono::steady_clock::now().time_since_epoch().count();

            m_Directory =
                std::filesystem::temp_directory_path() /
                (std::string{"signum_logger_factory_"} +
                 testing::UnitTest::GetInstance()->current_test_info()->name() +
                 "_" + std::to_string(timestamp));

            ASSERT_TRUE(std::filesystem::create_directories(m_Directory));

            m_Path = m_Directory / "signum.log";
        }

        auto TearDown() -> void override {
            std::error_code error;
            static_cast<void>(std::filesystem::remove_all(m_Directory, error));
        }
    };
} // namespace

TEST_F(LoggerFactoryTests, AcceptsConfigurationWithoutDestinations) {
    Logging::LoggingConfiguration configuration{
        .terminal_enabled = false,
        .minimum_level = Logging::LogLevel::Warning,
    };

    auto logger = Logging::makeLogger(configuration);

    EXPECT_EQ(logger.minimumLevel(), Logging::LogLevel::Warning);

    EXPECT_NO_THROW(
        logger.log(Logging::LogLevel::Warning, "Core", "No destination"));

    EXPECT_FALSE(std::filesystem::exists(m_Path));
}

TEST_F(LoggerFactoryTests, DefaultConfigurationWritesToTerminal) {
    ScopedClogCapture capture;

    {
        const Logging::LoggingConfiguration configuration;
        auto logger = Logging::makeLogger(configuration);

        EXPECT_EQ(logger.minimumLevel(), Logging::LogLevel::Info);

        logger.log(Logging::LogLevel::Info, "Core", "Terminal message");
    }

    EXPECT_NE(capture.text().find("[Info] [Core] Terminal message\n"),
              std::string::npos);
}

TEST_F(LoggerFactoryTests, FileOnlyConfigurationWritesToFile) {
    Logging::LoggingConfiguration configuration;
    configuration.terminal_enabled = false;
    configuration.file_path = m_Path;

    {
        auto logger = Logging::makeLogger(configuration);
        logger.log(Logging::LogLevel::Info, "Core", "File-only message");
    }

    ASSERT_TRUE(std::filesystem::exists(m_Path));

    EXPECT_NE(readFile(m_Path).find("[Info] [Core] File-only message\n"),
              std::string::npos);
}

TEST_F(LoggerFactoryTests, BothDestinationsReceiveSameMessage) {
    Logging::LoggingConfiguration configuration;
    configuration.file_path = m_Path;

    ScopedClogCapture capture;

    {
        auto logger = Logging::makeLogger(configuration);

        logger.log(Logging::LogLevel::Error, "Renderer", "Both destinations");
    }

    const std::string expected = "[Error] [Renderer] Both destinations\n";

    EXPECT_NE(capture.text().find(expected), std::string::npos);

    ASSERT_TRUE(std::filesystem::exists(m_Path));
    EXPECT_NE(readFile(m_Path).find(expected), std::string::npos);
}

TEST_F(LoggerFactoryTests, AppliesConfiguredMinimumLevel) {
    Logging::LoggingConfiguration configuration;
    configuration.terminal_enabled = false;
    configuration.file_path = m_Path;
    configuration.minimum_level = Logging::LogLevel::Warning;

    {
        auto logger = Logging::makeLogger(configuration);

        EXPECT_EQ(logger.minimumLevel(), Logging::LogLevel::Warning);

        logger.log(Logging::LogLevel::Info, "Core", "Filtered message");
        logger.log(Logging::LogLevel::Warning, "Core", "Accepted message");
    }

    const std::string content = readFile(m_Path);

    EXPECT_EQ(content.find("Filtered message"), std::string::npos);
    EXPECT_NE(content.find("[Warning] [Core] Accepted message\n"),
              std::string::npos);
}

TEST_F(LoggerFactoryTests, AppliesConfiguredFileRotationPolicy) {
    Logging::LoggingConfiguration configuration;
    configuration.terminal_enabled = false;
    configuration.file_path = m_Path;
    configuration.file_rotation_policy = {
        .max_file_size_bytes = 1U,
        .max_archives = 1U,
    };

    {
        auto logger = Logging::makeLogger(configuration);

        logger.log(Logging::LogLevel::Info, "Core", "First");
        logger.log(Logging::LogLevel::Info, "Core", "Second");
    }

    auto archive_path = m_Path;
    archive_path += ".1";

    ASSERT_TRUE(std::filesystem::exists(m_Path));
    ASSERT_TRUE(std::filesystem::exists(archive_path));

    EXPECT_NE(readFile(archive_path).find("[Info] [Core] First\n"),
              std::string::npos);

    EXPECT_NE(readFile(m_Path).find("[Info] [Core] Second\n"),
              std::string::npos);
}

TEST_F(LoggerFactoryTests, PropagatesInvalidRotationPolicy) {
    Logging::LoggingConfiguration configuration;
    configuration.terminal_enabled = false;
    configuration.file_path = m_Path;
    configuration.file_rotation_policy.max_file_size_bytes = 0U;

    EXPECT_THROW(static_cast<void>(Logging::makeLogger(configuration)),
                 std::invalid_argument);

    EXPECT_FALSE(std::filesystem::exists(m_Path));
}

TEST_F(LoggerFactoryTests, PropagatesFileOpenFailure) {
    Logging::LoggingConfiguration configuration;
    configuration.terminal_enabled = false;
    configuration.file_path = m_Directory / "missing_directory" / "signum.log";

    try {
        static_cast<void>(Logging::makeLogger(configuration));
        FAIL() << "Expected file initialization to fail";
    } catch (const Error::EngineError &error) {
        EXPECT_EQ(error.getCode(), Error::Code::FileLogOpenFailed);
        EXPECT_EQ(error.getOperation(), "Open Log File");
        EXPECT_TRUE(error.getNativeError().has_value());
    }
}
