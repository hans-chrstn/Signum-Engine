#include "engine/platform/event/platform_event_queue.hpp"
#include <gtest/gtest.h>
#include <variant>

namespace Platform = SNE::Engine::Platform;

TEST(PlatformEventQueueTests, StartsEmpty) {
    Platform::PlatformEventQueue queue;

    EXPECT_TRUE(queue.takeAll().empty());
}

TEST(PlatformEventQueueTests, PreservesEventArrivalOrder) {
    Platform::PlatformEventQueue queue;
    constexpr double kHorizontalScrollOffset = 0.5;
    constexpr double kVerticalScrollOffset = -1.25;

    queue.push(Platform::KeyEvent{
        .key = Platform::Key::W,
        .action = Platform::KeyAction::Press,
    });

    queue.push(Platform::ScrollEvent{
        .x_offset = kHorizontalScrollOffset,
        .y_offset = kVerticalScrollOffset,
    });

    queue.push(Platform::KeyEvent{
        .key = Platform::Key::W,
        .action = Platform::KeyAction::Release,
    });

    const auto events = queue.takeAll();

    ASSERT_EQ(events.size(), 3U);

    const auto *press = std::get_if<Platform::KeyEvent>(events.data());
    ASSERT_NE(press, nullptr);
    EXPECT_EQ(press->key, Platform::Key::W);
    EXPECT_EQ(press->action, Platform::KeyAction::Press);

    const auto *scroll = std::get_if<Platform::ScrollEvent>(&events[1]);
    ASSERT_NE(scroll, nullptr);
    EXPECT_DOUBLE_EQ(scroll->x_offset, kHorizontalScrollOffset);
    EXPECT_DOUBLE_EQ(scroll->y_offset, kVerticalScrollOffset);

    const auto *release = std::get_if<Platform::KeyEvent>(&events[2]);
    ASSERT_NE(release, nullptr);
    EXPECT_EQ(release->key, Platform::Key::W);
    EXPECT_EQ(release->action, Platform::KeyAction::Release);
}

TEST(PlatformEventQueueTests, TakeAllDrainsPendingEvents) {
    Platform::PlatformEventQueue queue;

    queue.push(Platform::WindowCloseRequestedEvent{});
    queue.push(Platform::WindowFocusEvent{.focused = false});

    const auto events = queue.takeAll();

    ASSERT_EQ(events.size(), 2U);
    EXPECT_TRUE(queue.takeAll().empty());
}

TEST(PlatformEventQueueTests, RemainsReusableAfterDraining) {
    Platform::PlatformEventQueue queue;

    queue.push(Platform::WindowCloseRequestedEvent{});

    const auto first_batch = queue.takeAll();

    ASSERT_EQ(first_batch.size(), 1U);

    queue.push(Platform::WindowFocusEvent{.focused = true});

    const auto second_batch = queue.takeAll();

    ASSERT_EQ(second_batch.size(), 1U);

    const auto *focus =
        std::get_if<Platform::WindowFocusEvent>(second_batch.data());

    ASSERT_NE(focus, nullptr);
    EXPECT_TRUE(focus->focused);

    EXPECT_TRUE(queue.takeAll().empty());
}

TEST(PlatformEventQueueTests, EmptyDrainDoesNotAffectLaterEvents) {
    Platform::PlatformEventQueue queue;

    EXPECT_TRUE(queue.takeAll().empty());

    queue.push(Platform::MouseButtonEvent{
        .button = Platform::MouseButton::Left,
        .action = Platform::MouseButtonAction::Press,
    });

    const auto events = queue.takeAll();

    ASSERT_EQ(events.size(), 1U);

    const auto *mouse = std::get_if<Platform::MouseButtonEvent>(events.data());

    ASSERT_NE(mouse, nullptr);
    EXPECT_EQ(mouse->button, Platform::MouseButton::Left);
    EXPECT_EQ(mouse->action, Platform::MouseButtonAction::Press);
}
