#include "engine/platform/event/platform_event.hpp"
#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include <GLFW/glfw3.h>
#include <array>
#include <cstddef>
#include <cstdlib>
#include <gtest/gtest.h>
#include <variant>

namespace Platform = SNE::Engine::Platform;

namespace {
    constexpr Platform::WindowSize kWindowSize{
        .width = 320,
        .height = 240,
    };

    constexpr int kNoModifiers = 0;
    constexpr int kUnsupportedAction = GLFW_REPEAT;

    constexpr double kCursorX = 125.5;
    constexpr double kCursorY = 240.25;

    struct MouseMapping {
        int native_button;
        Platform::MouseButton engine_button;
    };

    constexpr std::array kMouseMappings{
        MouseMapping{
            .native_button = GLFW_MOUSE_BUTTON_LEFT,
            .engine_button = Platform::MouseButton::Left,
        },
        MouseMapping{
            .native_button = GLFW_MOUSE_BUTTON_RIGHT,
            .engine_button = Platform::MouseButton::Right,
        },
        MouseMapping{
            .native_button = GLFW_MOUSE_BUTTON_MIDDLE,
            .engine_button = Platform::MouseButton::Middle,
        },
    };

    [[nodiscard]] auto hasDisplay() -> bool {
        return std::getenv("DISPLAY") != nullptr ||
               std::getenv("WAYLAND_DISPLAY") != nullptr;
    }

    [[nodiscard]] auto registeredMouseCallback(GLFWwindow *window)
        -> GLFWmousebuttonfun {
        GLFWmousebuttonfun callback =
            glfwSetMouseButtonCallback(window, nullptr);

        glfwSetMouseButtonCallback(window, callback);
        return callback;
    }

    [[nodiscard]] auto registeredCursorCallback(GLFWwindow *window)
        -> GLFWcursorposfun {
        GLFWcursorposfun callback = glfwSetCursorPosCallback(window, nullptr);

        glfwSetCursorPosCallback(window, callback);
        return callback;
    }
} // namespace

TEST(WindowMouseEventsIntegrationTests, TranslatesSupportedMouseButtons) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw_context{};

    Platform::Window window{
        kWindowSize,
        "Signum Mouse Button Translation Test",
    };

    GLFWmousebuttonfun callback =
        registeredMouseCallback(window.nativeHandle());

    ASSERT_NE(callback, nullptr);

    for (const MouseMapping &mapping : kMouseMappings) {
        callback(window.nativeHandle(), mapping.native_button, GLFW_PRESS,
                 kNoModifiers);
        callback(window.nativeHandle(), mapping.native_button, GLFW_RELEASE,
                 kNoModifiers);
    }

    const auto events = window.takeEvents();

    ASSERT_EQ(events.size(), kMouseMappings.size() * 2U);

    for (std::size_t index = 0; index < kMouseMappings.size(); ++index) {
        SCOPED_TRACE(kMouseMappings[index].native_button);

        const auto *press = std::get_if<Platform::MouseButtonEvent>(
            events.data() + (index * 2U));

        const auto *release = std::get_if<Platform::MouseButtonEvent>(
            events.data() + (index * 2U) + 1U);

        ASSERT_NE(press, nullptr);
        ASSERT_NE(release, nullptr);

        EXPECT_EQ(press->button, kMouseMappings[index].engine_button);
        EXPECT_EQ(press->action, Platform::MouseButtonAction::Press);

        EXPECT_EQ(release->button, kMouseMappings[index].engine_button);
        EXPECT_EQ(release->action, Platform::MouseButtonAction::Release);
    }

    EXPECT_TRUE(window.takeEvents().empty());
}

TEST(WindowMouseEventsIntegrationTests, IgnoresUnsupportedMouseInput) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw_context{};

    Platform::Window window{
        kWindowSize,
        "Signum Unsupported Mouse Input Test",
    };

    GLFWmousebuttonfun callback =
        registeredMouseCallback(window.nativeHandle());

    ASSERT_NE(callback, nullptr);

    callback(window.nativeHandle(), GLFW_MOUSE_BUTTON_4, GLFW_PRESS,
             kNoModifiers);

    callback(window.nativeHandle(), GLFW_MOUSE_BUTTON_LEFT, kUnsupportedAction,
             kNoModifiers);

    EXPECT_TRUE(window.takeEvents().empty());

    callback(window.nativeHandle(), GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS,
             kNoModifiers);

    const auto events = window.takeEvents();

    ASSERT_EQ(events.size(), 1U);

    const auto *event = std::get_if<Platform::MouseButtonEvent>(events.data());

    ASSERT_NE(event, nullptr);
    EXPECT_EQ(event->button, Platform::MouseButton::Left);
    EXPECT_EQ(event->action, Platform::MouseButtonAction::Press);
}

TEST(WindowMouseEventsIntegrationTests, PreservesCursorPositionAndEventOrder) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw_context{};

    Platform::Window window{
        kWindowSize,
        "Signum Cursor Event Translation Test",
    };

    GLFWmousebuttonfun mouse_callback =
        registeredMouseCallback(window.nativeHandle());

    GLFWcursorposfun cursor_callback =
        registeredCursorCallback(window.nativeHandle());

    ASSERT_NE(mouse_callback, nullptr);
    ASSERT_NE(cursor_callback, nullptr);

    mouse_callback(window.nativeHandle(), GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS,
                   kNoModifiers);

    cursor_callback(window.nativeHandle(), kCursorX, kCursorY);

    mouse_callback(window.nativeHandle(), GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE,
                   kNoModifiers);

    const auto events = window.takeEvents();

    ASSERT_EQ(events.size(), 3U);

    const auto *press = std::get_if<Platform::MouseButtonEvent>(events.data());

    const auto *cursor =
        std::get_if<Platform::CursorMovedEvent>(events.data() + 1U);

    const auto *release =
        std::get_if<Platform::MouseButtonEvent>(events.data() + 2U);

    ASSERT_NE(press, nullptr);
    ASSERT_NE(cursor, nullptr);
    ASSERT_NE(release, nullptr);

    EXPECT_EQ(press->action, Platform::MouseButtonAction::Press);

    EXPECT_DOUBLE_EQ(cursor->position.x, kCursorX);
    EXPECT_DOUBLE_EQ(cursor->position.y, kCursorY);

    EXPECT_EQ(release->action, Platform::MouseButtonAction::Release);

    EXPECT_TRUE(window.takeEvents().empty());
}
