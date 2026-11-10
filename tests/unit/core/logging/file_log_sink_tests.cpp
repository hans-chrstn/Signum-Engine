#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/logging/file_log_sink.hpp"
#include "engine/core/logging/log_record.hpp"
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

#include <gtest/gtest.h>

namespace Logging = SNE::Engine::Core::Logging;
namespace Error = SNE::Engine::Core::Error;

namespace {
    constexpr auto kTestTimestamp =
        std::chrono::sys_days{std::chrono::year{2026} / 11 / 8} +
        std::chrono::hours{18} + std::chrono::minutes{30} +
        std::chrono::seconds{15};

    [[nodiscard]] auto makeRecord(std::string_view message)
        -> Logging::LogRecord {
        return {
            .level = Logging::LogLevel::Info,
            .category = "Core",
            .message = std::string{message},
            .timestamp = kTestTimestamp,
        };
    }

    [[nodiscard]] auto expectedLine(std::string_view message) -> std::string {
        return std::string{"[2026-11-08T18:30:15Z] [Info] [Core] "} +
               std::string{message} + '\n';
    }

    [[nodiscard]] auto readFile(const std::filesystem::path &path)
        -> std::string {
        std::ifstream input{path, std::ios::binary};
        std::ostringstream output;
        output << input.rdbuf();
        return output.str();
    }

    class FileLogSinkTests : public testing::Test {
      protected:
        std::filesystem::path m_Directory;
        std::filesystem::path m_Path;

        auto SetUp() -> void override {
            const auto timestamp =
                std::chrono::steady_clock::now().time_since_epoch().count();

            m_Directory =
                std::filesystem::temp_directory_path() /
                (std::string{"signum_file_log_sink_"} +
                 testing::UnitTest::GetInstance()->current_test_info()->name() +
                 "_" + std::to_string(timestamp));

            ASSERT_TRUE(std::filesystem::create_directories(m_Directory));

            m_Path = m_Directory / "signum.log";
        }

        auto TearDown() -> void override {
            std::error_code error;
            static_cast<void>(std::filesystem::remove_all(m_Directory, error));
        }

        [[nodiscard]] auto archivePath(std::size_t index) const
            -> std::filesystem::path {
            auto path = m_Path;
            path += ".";
            path += std::to_string(index);
            return path;
        }
    };
} // namespace

TEST_F(FileLogSinkTests, WritesFormattedRecordToFile) {
    {
        Logging::FileLogSink sink{m_Path};
        sink.write(makeRecord("Engine initialized"));
    }

    ASSERT_TRUE(std::filesystem::exists(m_Path));

    EXPECT_EQ(readFile(m_Path), expectedLine("Engine initialized"));
    EXPECT_FALSE(std::filesystem::exists(archivePath(1U)));
}

TEST_F(FileLogSinkTests, AppendsToExistingLogFile) {
    {
        std::ofstream output{m_Path};
        ASSERT_TRUE(output.is_open());

        output << "Previous session\n";
        ASSERT_TRUE(output.good());
    }

    {
        Logging::FileLogSink sink{m_Path};
        sink.write(makeRecord("New session"));
    }

    EXPECT_EQ(readFile(m_Path),
              std::string{"Previous session\n"} + expectedLine("New session"));
}

TEST_F(FileLogSinkTests, RotatesBeforeWritingNextRecord) {
    const Logging::FileRotationPolicy policy{
        .max_file_size_bytes = 1U,
        .max_archives = 2U,
    };

    {
        Logging::FileLogSink sink{m_Path, policy};

        sink.write(makeRecord("First"));

        EXPECT_FALSE(std::filesystem::exists(archivePath(1U)));

        sink.write(makeRecord("Second"));
    }

    ASSERT_TRUE(std::filesystem::exists(m_Path));
    ASSERT_TRUE(std::filesystem::exists(archivePath(1U)));

    EXPECT_EQ(readFile(archivePath(1U)), expectedLine("First"));
    EXPECT_EQ(readFile(m_Path), expectedLine("Second"));
    EXPECT_FALSE(std::filesystem::exists(archivePath(2U)));
}

TEST_F(FileLogSinkTests, ShiftsArchivesAndRemovesOldest) {
    const Logging::FileRotationPolicy policy{
        .max_file_size_bytes = 1U,
        .max_archives = 2U,
    };

    {
        Logging::FileLogSink sink{m_Path, policy};

        sink.write(makeRecord("Record A"));
        sink.write(makeRecord("Record B"));
        sink.write(makeRecord("Record C"));
        sink.write(makeRecord("Record D"));
    }

    ASSERT_TRUE(std::filesystem::exists(m_Path));
    ASSERT_TRUE(std::filesystem::exists(archivePath(1U)));
    ASSERT_TRUE(std::filesystem::exists(archivePath(2U)));

    EXPECT_EQ(readFile(m_Path), expectedLine("Record D"));
    EXPECT_EQ(readFile(archivePath(1U)), expectedLine("Record C"));
    EXPECT_EQ(readFile(archivePath(2U)), expectedLine("Record B"));

    EXPECT_FALSE(std::filesystem::exists(archivePath(3U)));
}

TEST_F(FileLogSinkTests, ZeroRetentionTruncatesWithoutArchives) {
    const Logging::FileRotationPolicy policy{
        .max_file_size_bytes = 1U,
        .max_archives = 0U,
    };

    {
        Logging::FileLogSink sink{m_Path, policy};

        sink.write(makeRecord("First"));
        sink.write(makeRecord("Second"));
        sink.write(makeRecord("Third"));
    }

    ASSERT_TRUE(std::filesystem::exists(m_Path));

    EXPECT_EQ(readFile(m_Path), expectedLine("Third"));
    EXPECT_FALSE(std::filesystem::exists(archivePath(1U)));
}

TEST_F(FileLogSinkTests, RotatesOversizedExistingFileBeforeWriting) {
    {
        std::ofstream output{m_Path};
        ASSERT_TRUE(output.is_open());

        output << "Existing log data\n";
        ASSERT_TRUE(output.good());
    }

    const Logging::FileRotationPolicy policy{
        .max_file_size_bytes = 1U,
        .max_archives = 1U,
    };

    {
        Logging::FileLogSink sink{m_Path, policy};
        sink.write(makeRecord("New record"));
    }

    ASSERT_TRUE(std::filesystem::exists(archivePath(1U)));

    EXPECT_EQ(readFile(archivePath(1U)), "Existing log data\n");
    EXPECT_EQ(readFile(m_Path), expectedLine("New record"));
}

TEST_F(FileLogSinkTests, RejectsZeroMaximumFileSize) {
    const Logging::FileRotationPolicy policy{
        .max_file_size_bytes = 0U,
        .max_archives = 2U,
    };

    EXPECT_THROW(static_cast<void>(Logging::FileLogSink{m_Path, policy}),
                 std::invalid_argument);

    EXPECT_FALSE(std::filesystem::exists(m_Path));
}

TEST_F(FileLogSinkTests, ReportsFailureToOpenLogFile) {
    const auto invalid_path = m_Directory / "missing_directory" / "signum.log";

    try {
        Logging::FileLogSink sink{invalid_path};
        FAIL() << "Expected EngineError for unavailable log directory";
    } catch (const Error::EngineError &error) {
        EXPECT_EQ(error.getCode(), Error::Code::FileLogOpenFailed);
        EXPECT_EQ(error.getOperation(), "Open Log File");
        EXPECT_TRUE(error.getNativeError().has_value());
    }
}

TEST_F(FileLogSinkTests, RejectsFurtherWritesAfterRotationFailure) {
    const Logging::FileRotationPolicy policy{
        .max_file_size_bytes = 1U,
        .max_archives = 1U,
    };

    {
        Logging::FileLogSink sink{m_Path, policy};

        sink.write(makeRecord("Original record"));

        const auto blocked_archive = archivePath(1U);

        ASSERT_TRUE(std::filesystem::create_directory(blocked_archive));

        {
            std::ofstream marker{blocked_archive / "marker.txt"};
            ASSERT_TRUE(marker.is_open());
            marker << "Prevent archive removal";
        }

        try {
            sink.write(makeRecord("Triggers rotation"));
            FAIL() << "Expected rotation to fail";
        } catch (const Error::EngineError &error) {
            EXPECT_EQ(error.getCode(), Error::Code::FileLogRotationFailed);
            EXPECT_EQ(error.getOperation(), "Rotate Log File");
        }

        try {
            sink.write(makeRecord("Must be rejected"));
            FAIL() << "Expected fail-closed behavior";
        } catch (const Error::EngineError &error) {
            EXPECT_EQ(error.getCode(), Error::Code::FileLogRotationFailed);
            EXPECT_EQ(error.getOperation(), "Write Log File");
        }
    }

    EXPECT_EQ(readFile(m_Path), expectedLine("Original record"));
}
