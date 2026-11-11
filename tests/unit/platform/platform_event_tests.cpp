#include "engine/platform/event/platform_event.hpp"
#include <cstddef>
#include <gtest/gtest.h>
#include <variant>

namespace Platform = SNE::Engine::Platform;

constexpr std::size_t kPlatformEventSize = 8U;
static_assert(std::variant_size_v<Platform::PlatformEvent> ==
              kPlatformEventSize);

TEST(PlatformEventTests, PreservesKeyboardEventData) {
    const Platform::PlatformEvent event = Platform::KeyEvent{
        .key = Platform::Key::W,
        .action = Platform::KeyAction::Press,
    };

    const auto *key_event = std::get_if<Platform::KeyEvent>(&event);

    ASSERT_NE(key_event, nullptr);
    EXPECT_EQ(key_event->key, Platform::Key::W);
    EXPECT_EQ(key_event->action, Platform::KeyAction::Press);
}

TEST(PlatformEventTests, PreservesMouseButtonEventData) {
    const Platform::PlatformEvent event = Platform::MouseButtonEvent{
        .button = Platform::MouseButton::Right,
        .action = Platform::MouseButtonAction::Release,
    };

    const auto *mouse_event = std::get_if<Platform::MouseButtonEvent>(&event);

    ASSERT_NE(mouse_event, nullptr);
    EXPECT_EQ(mouse_event->button, Platform::MouseButton::Right);
    EXPECT_EQ(mouse_event->action, Platform::MouseButtonAction::Release);
}

TEST(PlatformEventTests, PreservesCursorAndScrollData) {
    const Platform::PlatformEvent cursor = Platform::CursorMovedEvent{
        .position = {.x = 125.5, .y = 240.25},
    };

    const auto *cursor_event = std::get_if<Platform::CursorMovedEvent>(&cursor);

    ASSERT_NE(cursor_event, nullptr);
    EXPECT_DOUBLE_EQ(cursor_event->position.x, 125.5);
    EXPECT_DOUBLE_EQ(cursor_event->position.y, 240.25);

    const Platform::PlatformEvent scroll = Platform::ScrollEvent{
        .x_offset = -0.5,
        .y_offset = 1.25,
    };

    const auto *scroll_event = std::get_if<Platform::ScrollEvent>(&scroll);

    ASSERT_NE(scroll_event, nullptr);
    EXPECT_DOUBLE_EQ(scroll_event->x_offset, -0.5);
    EXPECT_DOUBLE_EQ(scroll_event->y_offset, 1.25);
}

TEST(PlatformEventTests, PreservesWindowEventData) {
    const Platform::PlatformEvent resized = Platform::WindowResizedEvent{
        .width = 1280,
        .height = 720,
    };

    const auto *window_event =
        std::get_if<Platform::WindowResizedEvent>(&resized);

    ASSERT_NE(window_event, nullptr);
    EXPECT_EQ(window_event->width, 1280);
    EXPECT_EQ(window_event->height, 720);

    const Platform::PlatformEvent framebuffer =
        Platform::FramebufferResizedEvent{
            .width = 2560,
            .height = 1440,
    };

    const auto *framebuffer_event =
        std::get_if<Platform::FramebufferResizedEvent>(&framebuffer);

    ASSERT_NE(framebuffer_event, nullptr);
    EXPECT_EQ(framebuffer_event->width, 2560);
    EXPECT_EQ(framebuffer_event->height, 1440);

    const Platform::PlatformEvent focus =
        Platform::WindowFocusEvent{.focused = false};

    const auto *focus_event = std::get_if<Platform::WindowFocusEvent>(&focus);

    ASSERT_NE(focus_event, nullptr);
    EXPECT_FALSE(focus_event->focused);

    const Platform::PlatformEvent close = Platform::WindowCloseRequestedEvent{};

    EXPECT_TRUE(
        std::holds_alternative<Platform::WindowCloseRequestedEvent>(close));
}
