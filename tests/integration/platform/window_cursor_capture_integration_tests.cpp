#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <gtest/gtest.h>

namespace Platform = SNE::Engine::Platform;

namespace {
    constexpr int width = 320;
    constexpr int height = 240;
} // namespace

TEST(WindowCursorCaptureIntegrationTests, CapturesAndReleasesCursor) {
    if (std::getenv("DISPLAY") == nullptr &&
        std::getenv("WAYLAND_DISPLAY") == nullptr) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw_context{};

    Platform::Window window{
        {.width = width, .height = height},
        "Signum Cursor Capture Integration Test",
    };

    EXPECT_EQ(glfwGetInputMode(window.nativeHandle(), GLFW_CURSOR),
              GLFW_CURSOR_NORMAL);

    window.setCursorCaptured(true);

    EXPECT_EQ(glfwGetInputMode(window.nativeHandle(), GLFW_CURSOR),
              GLFW_CURSOR_DISABLED);

    window.setCursorCaptured(false);

    EXPECT_EQ(glfwGetInputMode(window.nativeHandle(), GLFW_CURSOR),
              GLFW_CURSOR_NORMAL);
}
