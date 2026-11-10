#include "engine/core/application.hpp"
#include "engine/core/application_configuration.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_discovery.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_selection.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_surface.hpp"
#include "engine/renderer/vulkan/vulkan_instance.hpp"
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

namespace Core = SNE::Engine::Core;
namespace Error = SNE::Engine::Core::Error;
namespace Logging = SNE::Engine::Core::Logging;
namespace Platform = SNE::Engine::Platform;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    constexpr Platform::WindowSize kWindowSize{
        .width = 640,
        .height = 480,
    };

    [[nodiscard]] auto hasDisplay() -> bool {
        return std::getenv("DISPLAY") != nullptr ||
               std::getenv("WAYLAND_DISPLAY") != nullptr;
    }

    [[nodiscard]] auto hasSuitableDevice(const Platform::Window &window)
        -> bool {
        Vulkan::VulkanInstance instance{
            "Signum Application Logging Integration Test",
            false,
        };

        Vulkan::VulkanSurface surface{
            instance.nativeHandle(),
            window.nativeHandle(),
        };

        const auto devices =
            Vulkan::enumeratePhysicalDevices(instance.nativeHandle());

        const auto discovered =
            Vulkan::inspectPhysicalDevices(devices, surface.nativeHandle());

        return Vulkan::selectPhysicalDevice(discovered).has_value();
    }

    [[nodiscard]] auto readFile(const std::filesystem::path &path)
        -> std::string {
        std::ifstream input{path, std::ios::binary};
        std::ostringstream output;

        output << input.rdbuf();

        return output.str();
    }

    class ApplicationLoggingIntegrationTests : public testing::Test {
      protected:
        std::filesystem::path m_Directory;
        std::filesystem::path m_Path;

        auto SetUp() -> void override {
            const auto timestamp =
                std::chrono::steady_clock::now().time_since_epoch().count();

            m_Directory =
                std::filesystem::temp_directory_path() /
                (std::string{"signum_application_logging_"} +
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

TEST_F(ApplicationLoggingIntegrationTests,
       CapturesPlatformAndApplicationStartupDiagnostics) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    {
        Platform::GlfwContext glfw{};

        const Platform::Window window{
            kWindowSize,
            "Signum Application Logging Integration Test",
        };

        if (!hasSuitableDevice(window)) {
            GTEST_SKIP() << "No suitable presentation-capable Vulkan device";
        }
    }

    Core::ApplicationConfiguration configuration{};
    configuration.application_name =
        "Signum Application Logging Integration Test";
    configuration.initial_window_size = kWindowSize;
    configuration.development_diagnostics_enabled = false;

    configuration.logging.terminal_enabled = false;
    configuration.logging.file_path = m_Path;
    configuration.logging.minimum_level = Logging::LogLevel::Info;

    {
        Core::Application application{std::move(configuration)};
    }

    ASSERT_TRUE(std::filesystem::exists(m_Path));

    const std::string content = readFile(m_Path);

    const auto platform_position =
        content.find("[Info] [Platform] GLFW initialized\n");

    const auto application_position =
        content.find("[Info] [Core] Application Initialized\n");

    ASSERT_NE(platform_position, std::string::npos);
    ASSERT_NE(application_position, std::string::npos);

    EXPECT_LT(platform_position, application_position);
}

TEST_F(ApplicationLoggingIntegrationTests,
       FileInitializationFailurePropagatesFromApplication) {
    Core::ApplicationConfiguration configuration{};

    configuration.logging.terminal_enabled = false;
    configuration.logging.file_path =
        m_Directory / "missing_directory" / "signum.log";

    try {
        Core::Application application{std::move(configuration)};

        FAIL() << "Expected application logging initialization to fail";
    } catch (const Error::EngineError &error) {
        EXPECT_EQ(error.getCode(), Error::Code::FileLogOpenFailed);
        EXPECT_EQ(error.getOperation(), "Open Log File");
        EXPECT_TRUE(error.getNativeError().has_value());
    }

    EXPECT_FALSE(std::filesystem::exists(m_Path));
}

TEST_F(ApplicationLoggingIntegrationTests,
       RejectsInvalidFileRotationPolicyDuringInitialization) {
    Core::ApplicationConfiguration configuration{};

    configuration.logging.terminal_enabled = false;
    configuration.logging.file_path = m_Path;
    configuration.logging.file_rotation_policy.max_file_size_bytes = 0U;

    EXPECT_THROW(static_cast<void>(Core::Application{std::move(configuration)}),
                 std::invalid_argument);

    EXPECT_FALSE(std::filesystem::exists(m_Path));
}
