#include "engine/platform/event/platform_event.hpp"
#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <gtest/gtest.h>
#include <variant>

namespace Platform = SNE::Engine::Platform;

namespace {
    constexpr Platform::WindowSize kWindowSize{
        .width = 320,
        .height = 240,
    };

    constexpr double kHorizontalScrollOffset = 0.5;
    constexpr double kVerticalScrollOffset = -1.25;

    [[nodiscard]] auto registeredScrollCallback(GLFWwindow *window)
        -> GLFWscrollfun {
        GLFWscrollfun callback = glfwSetScrollCallback(window, nullptr);
        glfwSetScrollCallback(window, callback);
        return callback;
    }
} // namespace

TEST(WindowScrollIntegrationTests, TranslatesScrollCallbackIntoPlatformEvent) {
    if (std::getenv("DISPLAY") == nullptr &&
        std::getenv("WAYLAND_DISPLAY") == nullptr) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw_context{};

    Platform::Window window{
        kWindowSize,
        "Signum Window Scroll Integration Test",
    };

    GLFWscrollfun callback = registeredScrollCallback(window.nativeHandle());

    ASSERT_NE(callback, nullptr);
    EXPECT_TRUE(window.takeEvents().empty());

    callback(window.nativeHandle(), kHorizontalScrollOffset,
             kVerticalScrollOffset);

    const auto events = window.takeEvents();

    ASSERT_EQ(events.size(), 1U);

    const auto *scroll = std::get_if<Platform::ScrollEvent>(events.data());

    ASSERT_NE(scroll, nullptr);
    EXPECT_DOUBLE_EQ(scroll->x_offset, kHorizontalScrollOffset);
    EXPECT_DOUBLE_EQ(scroll->y_offset, kVerticalScrollOffset);

    EXPECT_TRUE(window.takeEvents().empty());
}

TEST(WindowScrollIntegrationTests, KeepsEventsSeparateBetweenWindows) {
    if (std::getenv("DISPLAY") == nullptr &&
        std::getenv("WAYLAND_DISPLAY") == nullptr) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw_context{};

    Platform::Window first_window{
        kWindowSize,
        "Signum First Window",
    };

    Platform::Window second_window{
        kWindowSize,
        "Signum Second Window",
    };

    GLFWscrollfun callback =
        registeredScrollCallback(first_window.nativeHandle());

    ASSERT_NE(callback, nullptr);

    callback(first_window.nativeHandle(), kHorizontalScrollOffset,
             kVerticalScrollOffset);

    const auto first_events = first_window.takeEvents();

    ASSERT_EQ(first_events.size(), 1U);

    const auto *scroll =
        std::get_if<Platform::ScrollEvent>(first_events.data());

    ASSERT_NE(scroll, nullptr);
    EXPECT_DOUBLE_EQ(scroll->x_offset, kHorizontalScrollOffset);
    EXPECT_DOUBLE_EQ(scroll->y_offset, kVerticalScrollOffset);

    EXPECT_TRUE(second_window.takeEvents().empty());
}
