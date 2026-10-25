#include "engine/platform/glfw_context.hpp"
#include "engine/platform/input/key.hpp"
#include "engine/platform/window.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <cstdlib>
#include <gtest/gtest.h>

namespace Platform = SNE::Engine::Platform;

namespace {
    constexpr int width = 320;
    constexpr int height = 240;
} // namespace

TEST(WindowKeyboardIntegrationTests, StartsWithSupportedKeysReleased) {
    if (std::getenv("DISPLAY") == nullptr &&
        std::getenv("WAYLAND_DISPLAY") == nullptr) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw_context{};

    Platform::Window window{
        {.width = width, .height = height},
        "Signum Keyboard Integration Test",
    };

    constexpr std::array keys{
        Platform::Key::W,         Platform::Key::S,   Platform::Key::A,
        Platform::Key::D,         Platform::Key::Q,   Platform::Key::E,
        Platform::Key::LeftShift, Platform::Key::Tab,
    };

    for (const Platform::Key key : keys) {
        SCOPED_TRACE(static_cast<int>(key));
        EXPECT_FALSE(window.isKeyPressed(key));
    }
}

TEST(WindowKeyboardIntegrationTests, RejectsUnsupportedKey) {
    if (std::getenv("DISPLAY") == nullptr &&
        std::getenv("WAYLAND_DISPLAY") == nullptr) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw_context{};

    Platform::Window window{
        {.width = width, .height = height},
        "Signum Keyboard Integration Test",
    };

    constexpr auto kInvalidKey =
        std::bit_cast<Platform::Key>(std::uint8_t{255});

    EXPECT_DEATH(static_cast<void>(window.isKeyPressed(kInvalidKey)),
                 "Window key query requires a supported Key value");
}
