#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <gtest/gtest.h>

namespace Platform = SNE::Engine::Platform;

namespace {
    constexpr Platform::WindowSize kWindowSize{
        .width = 320,
        .height = 240,
    };
} // namespace

TEST(WindowCloseIntegrationTests, RequestsCloseWithoutDestroyingWindow) {
    if (std::getenv("DISPLAY") == nullptr &&
        std::getenv("WAYLAND_DISPLAY") == nullptr) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw_context{};

    Platform::Window window{
        kWindowSize,
        "Signum Window Close Integration Test",
    };

    ASSERT_FALSE(window.shouldClose());

    window.requestClose();

    EXPECT_TRUE(window.shouldClose());

    EXPECT_NE(window.nativeHandle(), nullptr);

    EXPECT_EQ(glfwWindowShouldClose(window.nativeHandle()), GLFW_TRUE);
}
