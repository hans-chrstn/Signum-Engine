#include "engine/platform/event/platform_event.hpp"
#include "engine/platform/glfw_context.hpp"
#include "engine/platform/input/key.hpp"
#include "engine/platform/window.hpp"
#include <GLFW/glfw3.h>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <gtest/gtest.h>
#include <variant>

namespace Platform = SNE::Engine::Platform;

namespace {
    constexpr int width = 320;
    constexpr int height = 240;

    constexpr int kUnusedScancode = 0;
    constexpr int kNoModifiers = 0;
    constexpr int kUnsupportedAction = -1;

    struct KeyMapping {
        int glfw_key;
        Platform::Key engine_key;
    };

    constexpr std::array kKeyMappings{
        KeyMapping{
            .glfw_key = GLFW_KEY_W,
            .engine_key = Platform::Key::W,
        },
        KeyMapping{
            .glfw_key = GLFW_KEY_S,
            .engine_key = Platform::Key::S,
        },
        KeyMapping{
            .glfw_key = GLFW_KEY_A,
            .engine_key = Platform::Key::A,
        },
        KeyMapping{
            .glfw_key = GLFW_KEY_D,
            .engine_key = Platform::Key::D,
        },
        KeyMapping{
            .glfw_key = GLFW_KEY_Q,
            .engine_key = Platform::Key::Q,
        },
        KeyMapping{
            .glfw_key = GLFW_KEY_E,
            .engine_key = Platform::Key::E,
        },
        KeyMapping{
            .glfw_key = GLFW_KEY_LEFT_SHIFT,
            .engine_key = Platform::Key::LeftShift,
        },
        KeyMapping{
            .glfw_key = GLFW_KEY_TAB,
            .engine_key = Platform::Key::Tab,
        },
    };

    [[nodiscard]] auto registeredKeyCallback(GLFWwindow *window) -> GLFWkeyfun {
        GLFWkeyfun callback = glfwSetKeyCallback(window, nullptr);
        glfwSetKeyCallback(window, callback);
        return callback;
    }
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

TEST(WindowKeyboardIntegrationTests, TranslatesAllSupportedKeys) {
    if (std::getenv("DISPLAY") == nullptr &&
        std::getenv("WAYLAND_DISPLAY") == nullptr) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw_context{};

    Platform::Window window{
        {.width = width, .height = height},
        "Signum Keyboard Event Translation Test",
    };

    GLFWkeyfun callback = registeredKeyCallback(window.nativeHandle());

    ASSERT_NE(callback, nullptr);

    for (const KeyMapping &mapping : kKeyMappings) {
        callback(window.nativeHandle(), mapping.glfw_key, kUnusedScancode,
                 GLFW_PRESS, kNoModifiers);
    }

    const auto events = window.takeEvents();

    ASSERT_EQ(events.size(), kKeyMappings.size());

    for (std::size_t index = 0; index < kKeyMappings.size(); ++index) {
        SCOPED_TRACE(kKeyMappings[index].glfw_key);

        const auto *event =
            std::get_if<Platform::KeyEvent>(events.data() + index);

        ASSERT_NE(event, nullptr);
        EXPECT_EQ(event->key, kKeyMappings[index].engine_key);
        EXPECT_EQ(event->action, Platform::KeyAction::Press);
    }

    EXPECT_TRUE(window.takeEvents().empty());
}

TEST(WindowKeyboardIntegrationTests, PreservesPressRepeatReleaseOrder) {
    if (std::getenv("DISPLAY") == nullptr &&
        std::getenv("WAYLAND_DISPLAY") == nullptr) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw_context{};

    Platform::Window window{
        {.width = width, .height = height},
        "Signum Keyboard Action Translation Test",
    };

    GLFWkeyfun callback = registeredKeyCallback(window.nativeHandle());

    ASSERT_NE(callback, nullptr);

    constexpr std::array kNativeActions{
        GLFW_PRESS,
        GLFW_REPEAT,
        GLFW_RELEASE,
    };

    constexpr std::array kExpectedActions{
        Platform::KeyAction::Press,
        Platform::KeyAction::Repeat,
        Platform::KeyAction::Release,
    };

    for (const int action : kNativeActions) {
        callback(window.nativeHandle(), GLFW_KEY_W, kUnusedScancode, action,
                 kNoModifiers);
    }

    const auto events = window.takeEvents();

    ASSERT_EQ(events.size(), kExpectedActions.size());

    for (std::size_t index = 0; index < kExpectedActions.size(); ++index) {
        const auto *event =
            std::get_if<Platform::KeyEvent>(events.data() + index);

        ASSERT_NE(event, nullptr);
        EXPECT_EQ(event->key, Platform::Key::W);
        EXPECT_EQ(event->action, kExpectedActions[index]);
    }
}

TEST(WindowKeyboardIntegrationTests, IgnoresUnsupportedKeysAndActions) {
    if (std::getenv("DISPLAY") == nullptr &&
        std::getenv("WAYLAND_DISPLAY") == nullptr) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw_context{};

    Platform::Window window{
        {.width = width, .height = height},
        "Signum Keyboard Unsupported Input Test",
    };

    GLFWkeyfun callback = registeredKeyCallback(window.nativeHandle());

    ASSERT_NE(callback, nullptr);

    callback(window.nativeHandle(), GLFW_KEY_ESCAPE, kUnusedScancode,
             GLFW_PRESS, kNoModifiers);

    callback(window.nativeHandle(), GLFW_KEY_W, kUnusedScancode,
             kUnsupportedAction, kNoModifiers);

    EXPECT_TRUE(window.takeEvents().empty());

    callback(window.nativeHandle(), GLFW_KEY_A, kUnusedScancode, GLFW_PRESS,
             kNoModifiers);

    const auto events = window.takeEvents();

    ASSERT_EQ(events.size(), 1U);

    const auto *event = std::get_if<Platform::KeyEvent>(events.data());

    ASSERT_NE(event, nullptr);
    EXPECT_EQ(event->key, Platform::Key::A);
    EXPECT_EQ(event->action, Platform::KeyAction::Press);
}
