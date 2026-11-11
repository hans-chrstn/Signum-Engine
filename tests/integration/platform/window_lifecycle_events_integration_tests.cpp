#include "engine/platform/event/platform_event.hpp"
#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <gtest/gtest.h>
#include <variant>

namespace Platform = SNE::Engine::Platform;

namespace {
    constexpr Platform::WindowSize kInitialWindowSize{
        .width = 320,
        .height = 240,
    };

    constexpr Platform::WindowSize kUpdatedWindowSize{
        .width = 800,
        .height = 600,
    };

    constexpr Platform::FramebufferSize kUpdatedFramebufferSize{
        .width = 1600,
        .height = 1200,
    };

    constexpr Platform::FramebufferSize kUnavailableFramebufferSize{
        .width = 0,
        .height = 0,
    };

    struct LifecycleCallbacks {
        GLFWwindowsizefun window_size = nullptr;
        GLFWframebuffersizefun framebuffer_size = nullptr;
        GLFWwindowfocusfun focus = nullptr;
        GLFWwindowclosefun close = nullptr;
    };

    [[nodiscard]] auto hasDisplay() -> bool {
        return std::getenv("DISPLAY") != nullptr ||
               std::getenv("WAYLAND_DISPLAY") != nullptr;
    }

    [[nodiscard]] auto registeredLifecycleCallbacks(GLFWwindow *window)
        -> LifecycleCallbacks {
        const GLFWwindowsizefun window_size =
            glfwSetWindowSizeCallback(window, nullptr);
        glfwSetWindowSizeCallback(window, window_size);

        const GLFWframebuffersizefun framebuffer_size =
            glfwSetFramebufferSizeCallback(window, nullptr);
        glfwSetFramebufferSizeCallback(window, framebuffer_size);

        const GLFWwindowfocusfun focus =
            glfwSetWindowFocusCallback(window, nullptr);
        glfwSetWindowFocusCallback(window, focus);

        const GLFWwindowclosefun close =
            glfwSetWindowCloseCallback(window, nullptr);
        glfwSetWindowCloseCallback(window, close);

        return LifecycleCallbacks{
            .window_size = window_size,
            .framebuffer_size = framebuffer_size,
            .focus = focus,
            .close = close,
        };
    }
} // namespace

TEST(WindowLifecycleEventsIntegrationTests, RegistersAllLifecycleCallbacks) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw_context{};

    Platform::Window window{
        kInitialWindowSize,
        "Signum Lifecycle Callback Registration Test",
    };

    const LifecycleCallbacks callbacks =
        registeredLifecycleCallbacks(window.nativeHandle());

    EXPECT_NE(callbacks.window_size, nullptr);
    EXPECT_NE(callbacks.framebuffer_size, nullptr);
    EXPECT_NE(callbacks.focus, nullptr);
    EXPECT_NE(callbacks.close, nullptr);

    EXPECT_TRUE(window.takeEvents().empty());
}

TEST(WindowLifecycleEventsIntegrationTests,
     TranslatesLogicalAndFramebufferSizeEvents) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw_context{};

    Platform::Window window{
        kInitialWindowSize,
        "Signum Window Resize Event Test",
    };

    const LifecycleCallbacks callbacks =
        registeredLifecycleCallbacks(window.nativeHandle());

    ASSERT_NE(callbacks.window_size, nullptr);
    ASSERT_NE(callbacks.framebuffer_size, nullptr);

    callbacks.window_size(window.nativeHandle(), kUpdatedWindowSize.width,
                          kUpdatedWindowSize.height);

    callbacks.framebuffer_size(window.nativeHandle(),
                               kUpdatedFramebufferSize.width,
                               kUpdatedFramebufferSize.height);

    const auto events = window.takeEvents();

    ASSERT_EQ(events.size(), 2U);

    const auto *logical =
        std::get_if<Platform::WindowResizedEvent>(events.data());

    const auto *framebuffer =
        std::get_if<Platform::FramebufferResizedEvent>(&events.at(1));

    ASSERT_NE(logical, nullptr);
    ASSERT_NE(framebuffer, nullptr);

    EXPECT_EQ(logical->width, kUpdatedWindowSize.width);
    EXPECT_EQ(logical->height, kUpdatedWindowSize.height);

    EXPECT_EQ(framebuffer->width, kUpdatedFramebufferSize.width);
    EXPECT_EQ(framebuffer->height, kUpdatedFramebufferSize.height);

    EXPECT_TRUE(window.takeEvents().empty());
}

TEST(WindowLifecycleEventsIntegrationTests,
     PreservesZeroFramebufferDimensions) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw_context{};

    Platform::Window window{
        kInitialWindowSize,
        "Signum Zero Framebuffer Size Test",
    };

    const LifecycleCallbacks callbacks =
        registeredLifecycleCallbacks(window.nativeHandle());

    ASSERT_NE(callbacks.framebuffer_size, nullptr);

    callbacks.framebuffer_size(window.nativeHandle(),
                               kUnavailableFramebufferSize.width,
                               kUnavailableFramebufferSize.height);

    const auto events = window.takeEvents();

    ASSERT_EQ(events.size(), 1U);

    const auto *framebuffer =
        std::get_if<Platform::FramebufferResizedEvent>(events.data());

    ASSERT_NE(framebuffer, nullptr);

    EXPECT_EQ(framebuffer->width, kUnavailableFramebufferSize.width);
    EXPECT_EQ(framebuffer->height, kUnavailableFramebufferSize.height);
}

TEST(WindowLifecycleEventsIntegrationTests, TranslatesFocusGainedAndLost) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw_context{};

    Platform::Window window{
        kInitialWindowSize,
        "Signum Window Focus Event Test",
    };

    const LifecycleCallbacks callbacks =
        registeredLifecycleCallbacks(window.nativeHandle());

    ASSERT_NE(callbacks.focus, nullptr);

    callbacks.focus(window.nativeHandle(), GLFW_FALSE);
    callbacks.focus(window.nativeHandle(), GLFW_TRUE);

    const auto events = window.takeEvents();

    ASSERT_EQ(events.size(), 2U);

    const auto *lost = std::get_if<Platform::WindowFocusEvent>(events.data());

    const auto *gained = std::get_if<Platform::WindowFocusEvent>(&events.at(1));

    ASSERT_NE(lost, nullptr);
    ASSERT_NE(gained, nullptr);

    EXPECT_FALSE(lost->focused);
    EXPECT_TRUE(gained->focused);

    EXPECT_TRUE(window.takeEvents().empty());
}

TEST(WindowLifecycleEventsIntegrationTests,
     QueuesCloseRequestWithoutChangingCloseFlag) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw_context{};

    Platform::Window window{
        kInitialWindowSize,
        "Signum Native Close Event Test",
    };

    const LifecycleCallbacks callbacks =
        registeredLifecycleCallbacks(window.nativeHandle());

    ASSERT_NE(callbacks.close, nullptr);
    ASSERT_FALSE(window.shouldClose());

    // Direct callback invocation does not simulate GLFW setting its flag.
    callbacks.close(window.nativeHandle());

    const auto events = window.takeEvents();

    ASSERT_EQ(events.size(), 1U);

    EXPECT_TRUE(std::holds_alternative<Platform::WindowCloseRequestedEvent>(
        events.front()));

    EXPECT_FALSE(window.shouldClose());
    EXPECT_TRUE(window.takeEvents().empty());
}

TEST(WindowLifecycleEventsIntegrationTests,
     ProgrammaticCloseDoesNotQueueNativeCloseEvent) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw_context{};

    Platform::Window window{
        kInitialWindowSize,
        "Signum Programmatic Close Event Test",
    };

    ASSERT_FALSE(window.shouldClose());

    window.requestClose();

    EXPECT_TRUE(window.shouldClose());
    EXPECT_TRUE(window.takeEvents().empty());
}

TEST(WindowLifecycleEventsIntegrationTests, PreservesLifecycleEventOrder) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw_context{};

    Platform::Window window{
        kInitialWindowSize,
        "Signum Lifecycle Event Ordering Test",
    };

    const LifecycleCallbacks callbacks =
        registeredLifecycleCallbacks(window.nativeHandle());

    ASSERT_NE(callbacks.window_size, nullptr);
    ASSERT_NE(callbacks.framebuffer_size, nullptr);
    ASSERT_NE(callbacks.focus, nullptr);
    ASSERT_NE(callbacks.close, nullptr);

    callbacks.focus(window.nativeHandle(), GLFW_FALSE);

    callbacks.window_size(window.nativeHandle(), kUpdatedWindowSize.width,
                          kUpdatedWindowSize.height);

    callbacks.framebuffer_size(window.nativeHandle(),
                               kUpdatedFramebufferSize.width,
                               kUpdatedFramebufferSize.height);

    callbacks.focus(window.nativeHandle(), GLFW_TRUE);

    callbacks.close(window.nativeHandle());

    const auto events = window.takeEvents();

    ASSERT_EQ(events.size(), 5U);

    EXPECT_TRUE(
        std::holds_alternative<Platform::WindowFocusEvent>(events.at(0)));

    EXPECT_TRUE(
        std::holds_alternative<Platform::WindowResizedEvent>(events.at(1)));

    EXPECT_TRUE(std::holds_alternative<Platform::FramebufferResizedEvent>(
        events.at(2)));

    EXPECT_TRUE(
        std::holds_alternative<Platform::WindowFocusEvent>(events.at(3)));

    EXPECT_TRUE(std::holds_alternative<Platform::WindowCloseRequestedEvent>(
        events.at(4)));

    EXPECT_TRUE(window.takeEvents().empty());
}

TEST(WindowLifecycleEventsIntegrationTests, KeepsEventsSeparateBetweenWindows) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw_context{};

    Platform::Window first_window{
        kInitialWindowSize,
        "Signum First Lifecycle Window",
    };

    Platform::Window second_window{
        kInitialWindowSize,
        "Signum Second Lifecycle Window",
    };

    const LifecycleCallbacks first_callbacks =
        registeredLifecycleCallbacks(first_window.nativeHandle());

    const LifecycleCallbacks second_callbacks =
        registeredLifecycleCallbacks(second_window.nativeHandle());

    ASSERT_NE(first_callbacks.focus, nullptr);
    ASSERT_NE(second_callbacks.window_size, nullptr);

    first_callbacks.focus(first_window.nativeHandle(), GLFW_FALSE);

    second_callbacks.window_size(second_window.nativeHandle(),
                                 kUpdatedWindowSize.width,
                                 kUpdatedWindowSize.height);

    const auto first_events = first_window.takeEvents();
    const auto second_events = second_window.takeEvents();

    ASSERT_EQ(first_events.size(), 1U);
    ASSERT_EQ(second_events.size(), 1U);

    const auto *focus =
        std::get_if<Platform::WindowFocusEvent>(first_events.data());

    const auto *resize =
        std::get_if<Platform::WindowResizedEvent>(second_events.data());

    ASSERT_NE(focus, nullptr);
    ASSERT_NE(resize, nullptr);

    EXPECT_FALSE(focus->focused);
    EXPECT_EQ(resize->width, kUpdatedWindowSize.width);
    EXPECT_EQ(resize->height, kUpdatedWindowSize.height);

    EXPECT_TRUE(first_window.takeEvents().empty());
    EXPECT_TRUE(second_window.takeEvents().empty());
}

TEST(WindowLifecycleEventsIntegrationTests,
     IgnoresCallbacksWithoutOwningWindowPointer) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw_context{};

    Platform::Window window{
        kInitialWindowSize,
        "Signum Lifecycle Null Owner Test",
    };

    const LifecycleCallbacks callbacks =
        registeredLifecycleCallbacks(window.nativeHandle());

    ASSERT_NE(callbacks.window_size, nullptr);
    ASSERT_NE(callbacks.framebuffer_size, nullptr);
    ASSERT_NE(callbacks.focus, nullptr);
    ASSERT_NE(callbacks.close, nullptr);

    GLFWwindow *native_window = window.nativeHandle();

    glfwSetWindowUserPointer(native_window, nullptr);

    callbacks.window_size(native_window, kUpdatedWindowSize.width,
                          kUpdatedWindowSize.height);

    callbacks.framebuffer_size(native_window, kUpdatedFramebufferSize.width,
                               kUpdatedFramebufferSize.height);

    callbacks.focus(native_window, GLFW_FALSE);
    callbacks.close(native_window);

    glfwSetWindowUserPointer(native_window, &window);

    EXPECT_TRUE(window.takeEvents().empty());
}
