#include "engine/core/application.hpp"
#include "engine/core/application_configuration.hpp"
#include "engine/platform/event/platform_event.hpp"
#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_discovery.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_selection.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_surface.hpp"
#include "engine/renderer/vulkan/vulkan_instance.hpp"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <gtest/gtest.h>
#include <span>
#include <utility>
#include <variant>

namespace Core = SNE::Engine::Core;
namespace Platform = SNE::Engine::Platform;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    constexpr Platform::WindowSize kWindowSize{
        .width = 640,
        .height = 480,
    };

    constexpr double kHorizontalOffset = 0.5;
    constexpr double kVerticalOffset = -1.25;

    [[nodiscard]] auto hasDisplay() -> bool {
        return std::getenv("DISPLAY") != nullptr ||
               std::getenv("WAYLAND_DISPLAY") != nullptr;
    }

    [[nodiscard]] auto hasSuitableDevice(const Platform::Window &window)
        -> bool {
        Vulkan::VulkanInstance instance{
            "Signum Application Platform Events Integration Test",
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
} // namespace

TEST(ApplicationPlatformEventsIntegrationTests,
     DeliversScrollEventsBeforeApplicationUpdate) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    {
        Platform::GlfwContext glfw_context{};

        Platform::Window window{
            kWindowSize,
            "Signum Application Platform Events Integration Test",
        };

        if (!hasSuitableDevice(window)) {
            GTEST_SKIP() << "No suitable presentation-capable Vulkan device";
        }
    }

    Core::ApplicationConfiguration configuration{};
    configuration.application_name =
        "Signum Application Platform Events Integration Test";
    configuration.initial_window_size = kWindowSize;
    configuration.development_diagnostics_enabled = false;
    configuration.logging.terminal_enabled = false;

    Core::Application application{std::move(configuration)};

    GLFWwindow *native_window = application.window().nativeHandle();

    GLFWscrollfun scroll_callback =
        glfwSetScrollCallback(native_window, nullptr);

    glfwSetScrollCallback(native_window, scroll_callback);

    ASSERT_NE(scroll_callback, nullptr);

    scroll_callback(native_window, kHorizontalOffset, kVerticalOffset);

    bool event_received{false};
    bool update_observed_event{false};

    application.run(
        [&](Core::Application &app) -> void {
            update_observed_event = event_received;
            app.requestClose();
        },
        {}, {},
        [&](Core::Application &app,
            std::span<const Platform::PlatformEvent> events) -> void {
            EXPECT_EQ(&app, &application);
            ASSERT_EQ(events.size(), 1U);

            const auto *scroll =
                std::get_if<Platform::ScrollEvent>(events.data());

            ASSERT_NE(scroll, nullptr);
            EXPECT_DOUBLE_EQ(scroll->x_offset, kHorizontalOffset);
            EXPECT_DOUBLE_EQ(scroll->y_offset, kVerticalOffset);

            event_received = true;
        });

    EXPECT_TRUE(event_received);
    EXPECT_TRUE(update_observed_event);
}
