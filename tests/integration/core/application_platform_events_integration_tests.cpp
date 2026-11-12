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
#include <cstddef>
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

    constexpr int kUnusedScancode = 0;
    constexpr int kNoModifiers = 0;

    constexpr Platform::CursorPosition kCursorStart{
        .x = 100.0,
        .y = 200.0,
    };

    constexpr Platform::CursorPosition kCursorEnd{
        .x = 125.0,
        .y = 195.0,
    };

    constexpr double kExpectedCursorDeltaX = 25.0;
    constexpr double kExpectedCursorDeltaY = -5.0;

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

            const auto scroll_delta = app.inputState().scrollDelta();

            EXPECT_DOUBLE_EQ(scroll_delta.x, kHorizontalOffset);
            EXPECT_DOUBLE_EQ(scroll_delta.y, kVerticalOffset);

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

            const auto scroll_delta = app.inputState().scrollDelta();

            EXPECT_DOUBLE_EQ(scroll_delta.x, kHorizontalOffset);
            EXPECT_DOUBLE_EQ(scroll_delta.y, kVerticalOffset);

            event_received = true;
        });

    EXPECT_TRUE(event_received);
    EXPECT_TRUE(update_observed_event);
}

TEST(ApplicationPlatformEventsIntegrationTests,
     UpdatesInputStateBeforeApplicationCallbacks) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    {
        Platform::GlfwContext glfw_context{};
        Platform::Window window{
            kWindowSize,
            "Signum Application Input State Integration Test",
        };

        if (!hasSuitableDevice(window)) {
            GTEST_SKIP() << "No suitable presentation-capable Vulkan device";
        }
    }

    Core::ApplicationConfiguration configuration{};
    configuration.application_name =
        "Signum Application Input State Integration Test";
    configuration.initial_window_size = kWindowSize;
    configuration.development_diagnostics_enabled = false;
    configuration.logging.terminal_enabled = false;

    Core::Application application{std::move(configuration)};

    GLFWwindow *native_window = application.window().nativeHandle();

    GLFWkeyfun key_callback = glfwSetKeyCallback(native_window, nullptr);
    glfwSetKeyCallback(native_window, key_callback);

    ASSERT_NE(key_callback, nullptr);

    key_callback(native_window, GLFW_KEY_W, kUnusedScancode, GLFW_PRESS,
                 kNoModifiers);

    bool event_callback_observed_press{false};
    bool update_observed_press{false};

    application.run(
        [&](Core::Application &app) -> void {
            update_observed_press =
                event_callback_observed_press &&
                app.inputState().isKeyDown(Platform::Key::W) &&
                app.inputState().wasKeyPressed(Platform::Key::W);

            app.requestClose();
        },
        {}, {},
        [&](Core::Application &app,
            std::span<const Platform::PlatformEvent> events) -> void {
            for (const Platform::PlatformEvent &event : events) {
                const auto *key_event = std::get_if<Platform::KeyEvent>(&event);

                if (key_event != nullptr &&
                    key_event->key == Platform::Key::W &&
                    key_event->action == Platform::KeyAction::Press) {
                    event_callback_observed_press =
                        app.inputState().isKeyDown(Platform::Key::W) &&
                        app.inputState().wasKeyPressed(Platform::Key::W);
                }
            }
        });

    EXPECT_TRUE(event_callback_observed_press);
    EXPECT_TRUE(update_observed_press);
}

TEST(ApplicationPlatformEventsIntegrationTests,
     MaintainsInputStateWithoutPlatformEventCallback) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    {
        Platform::GlfwContext glfw_context{};
        Platform::Window window{
            kWindowSize,
            "Signum Application Input Frame Integration Test",
        };

        if (!hasSuitableDevice(window)) {
            GTEST_SKIP() << "No suitable presentation-capable Vulkan device";
        }
    }

    Core::ApplicationConfiguration configuration{};
    configuration.application_name =
        "Signum Application Input Frame Integration Test";
    configuration.initial_window_size = kWindowSize;
    configuration.development_diagnostics_enabled = false;
    configuration.logging.terminal_enabled = false;

    Core::Application application{std::move(configuration)};

    GLFWwindow *native_window = application.window().nativeHandle();

    GLFWkeyfun key_callback = glfwSetKeyCallback(native_window, nullptr);
    glfwSetKeyCallback(native_window, key_callback);

    ASSERT_NE(key_callback, nullptr);

    key_callback(native_window, GLFW_KEY_W, kUnusedScancode, GLFW_PRESS,
                 kNoModifiers);

    std::size_t frame_index{0U};

    application.run([&](Core::Application &app) -> void {
        const Platform::InputState &input = app.inputState();

        switch (frame_index) {
        case 0U:
            EXPECT_TRUE(input.isKeyDown(Platform::Key::W));
            EXPECT_TRUE(input.wasKeyPressed(Platform::Key::W));
            EXPECT_FALSE(input.wasKeyReleased(Platform::Key::W));
            break;

        case 1U:
            EXPECT_TRUE(input.isKeyDown(Platform::Key::W));
            EXPECT_FALSE(input.wasKeyPressed(Platform::Key::W));
            EXPECT_FALSE(input.wasKeyReleased(Platform::Key::W));

            key_callback(native_window, GLFW_KEY_W, kUnusedScancode,
                         GLFW_RELEASE, kNoModifiers);
            break;

        case 2U:
            EXPECT_FALSE(input.isKeyDown(Platform::Key::W));
            EXPECT_FALSE(input.wasKeyPressed(Platform::Key::W));
            EXPECT_TRUE(input.wasKeyReleased(Platform::Key::W));

            app.requestClose();
            break;

        default:
            ADD_FAILURE() << "Unexpected additional application frame";
            app.requestClose();
            break;
        }

        ++frame_index;
    });

    EXPECT_EQ(frame_index, 3U);
}

TEST(ApplicationPlatformEventsIntegrationTests,
     UpdatesCursorStateBeforeApplicationCallbacks) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    {
        Platform::GlfwContext glfw_context{};

        Platform::Window window{
            kWindowSize,
            "Signum Application Cursor Integration Test",
        };

        if (!hasSuitableDevice(window)) {
            GTEST_SKIP() << "No suitable presentation-capable Vulkan device";
        }
    }

    Core::ApplicationConfiguration configuration{};
    configuration.application_name =
        "Signum Application Cursor Integration Test";
    configuration.initial_window_size = kWindowSize;
    configuration.development_diagnostics_enabled = false;
    configuration.logging.terminal_enabled = false;

    Core::Application application{std::move(configuration)};

    GLFWwindow *native_window = application.window().nativeHandle();

    GLFWcursorposfun cursor_callback =
        glfwSetCursorPosCallback(native_window, nullptr);

    glfwSetCursorPosCallback(native_window, cursor_callback);

    ASSERT_NE(cursor_callback, nullptr);

    cursor_callback(native_window, kCursorStart.x, kCursorStart.y);
    cursor_callback(native_window, kCursorEnd.x, kCursorEnd.y);

    bool event_callback_observed_cursor{false};
    bool update_observed_cursor{false};

    application.run(
        [&](Core::Application &app) -> void {
            const auto position = app.inputState().cursorPosition();

            ASSERT_TRUE(position.has_value());

            const Platform::CursorPosition actual_position =
                position.value_or(Platform::CursorPosition{});

            const auto delta = app.inputState().cursorDelta();

            EXPECT_DOUBLE_EQ(actual_position.x, kCursorEnd.x);
            EXPECT_DOUBLE_EQ(actual_position.y, kCursorEnd.y);
            EXPECT_DOUBLE_EQ(delta.x, kExpectedCursorDeltaX);
            EXPECT_DOUBLE_EQ(delta.y, kExpectedCursorDeltaY);

            update_observed_cursor = event_callback_observed_cursor;

            app.requestClose();
        },
        {}, {},
        [&](Core::Application &app,
            std::span<const Platform::PlatformEvent> events) -> void {
            EXPECT_EQ(&app, &application);

            ASSERT_EQ(events.size(), 2U);

            const auto *first_cursor =
                std::get_if<Platform::CursorMovedEvent>(events.data());

            const auto *second_cursor =
                std::get_if<Platform::CursorMovedEvent>(events.data() + 1);

            ASSERT_NE(first_cursor, nullptr);
            ASSERT_NE(second_cursor, nullptr);

            EXPECT_DOUBLE_EQ(first_cursor->position.x, kCursorStart.x);
            EXPECT_DOUBLE_EQ(first_cursor->position.y, kCursorStart.y);

            EXPECT_DOUBLE_EQ(second_cursor->position.x, kCursorEnd.x);
            EXPECT_DOUBLE_EQ(second_cursor->position.y, kCursorEnd.y);

            const auto position = app.inputState().cursorPosition();

            ASSERT_TRUE(position.has_value());

            const Platform::CursorPosition actual_position =
                position.value_or(Platform::CursorPosition{});

            const auto delta = app.inputState().cursorDelta();

            EXPECT_DOUBLE_EQ(actual_position.x, kCursorEnd.x);
            EXPECT_DOUBLE_EQ(actual_position.y, kCursorEnd.y);
            EXPECT_DOUBLE_EQ(delta.x, kExpectedCursorDeltaX);
            EXPECT_DOUBLE_EQ(delta.y, kExpectedCursorDeltaY);

            event_callback_observed_cursor = true;
        });

    EXPECT_TRUE(event_callback_observed_cursor);
    EXPECT_TRUE(update_observed_cursor);
}

TEST(ApplicationPlatformEventsIntegrationTests,
     ProvidesConsistentFrameTimingAcrossCallbacks) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    {
        Platform::GlfwContext glfw_context{};

        Platform::Window window{
            kWindowSize,
            "Signum Application Frame Timing Integration Test",
        };

        if (!hasSuitableDevice(window)) {
            GTEST_SKIP() << "No suitable presentation-capable Vulkan device";
        }
    }

    Core::ApplicationConfiguration configuration{};
    configuration.application_name =
        "Signum Application Frame Timing Integration Test";
    configuration.initial_window_size = kWindowSize;
    configuration.development_diagnostics_enabled = false;
    configuration.logging.terminal_enabled = false;

    Core::Application application{std::move(configuration)};

    GLFWwindow *native_window = application.window().nativeHandle();

    GLFWkeyfun key_callback = glfwSetKeyCallback(native_window, nullptr);

    glfwSetKeyCallback(native_window, key_callback);

    ASSERT_NE(key_callback, nullptr);

    key_callback(native_window, GLFW_KEY_W, kUnusedScancode, GLFW_PRESS,
                 kNoModifiers);

    constexpr std::size_t kExpectedFrameCount = 2U;

    std::size_t frame_index{0U};
    bool matching_event_received{false};
    Core::Time::Duration event_callback_delta{};

    application.run(
        [&](Core::Application &app) -> void {
            const Core::Time::Duration delta = app.frameDelta();

            EXPECT_TRUE(matching_event_received);
            EXPECT_DOUBLE_EQ(delta.count(), event_callback_delta.count());

            EXPECT_GE(delta.count(), Core::Time::Duration::zero().count());

            switch (frame_index) {
            case 0U:
                EXPECT_DOUBLE_EQ(delta.count(),
                                 Core::Time::Duration::zero().count());

                key_callback(native_window, GLFW_KEY_W, kUnusedScancode,
                             GLFW_RELEASE, kNoModifiers);
                break;

            case 1U:
                app.requestClose();
                break;

            default:
                ADD_FAILURE() << "Unexpected additional application frame";
                app.requestClose();
                break;
            }

            matching_event_received = false;
            ++frame_index;
        },
        {}, {},
        [&](Core::Application &app,
            std::span<const Platform::PlatformEvent> events) -> void {
            const Platform::KeyAction expected_action =
                frame_index == 0U ? Platform::KeyAction::Press
                                  : Platform::KeyAction::Release;

            for (const Platform::PlatformEvent &event : events) {
                const auto *key_event = std::get_if<Platform::KeyEvent>(&event);

                if (key_event != nullptr &&
                    key_event->key == Platform::Key::W &&
                    key_event->action == expected_action) {
                    matching_event_received = true;
                    event_callback_delta = app.frameDelta();
                }
            }
        });

    EXPECT_EQ(frame_index, kExpectedFrameCount);
}
