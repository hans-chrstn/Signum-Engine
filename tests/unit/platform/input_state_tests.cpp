#include "engine/platform/event/platform_event.hpp"
#include "engine/platform/input/input_state.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <gtest/gtest.h>
#include <span>

namespace Platform = SNE::Engine::Platform;

namespace {
    constexpr double kZeroScroll = 0.0;
    constexpr double kQuarterScroll = 0.25;
    constexpr double kHalfScroll = 0.5;
    constexpr double kThreeQuarterScroll = 0.75;
    constexpr double kOneScroll = 1.0;
    constexpr double kOneAndQuarterScroll = 1.25;
    constexpr double kTwoScroll = 2.0;

    constexpr Platform::CursorPosition kCursorOrigin{};

    constexpr Platform::CursorPosition kInitialCursorPosition{
        .x = 100.0,
        .y = 200.0,
    };

    constexpr Platform::CursorPosition kSecondCursorPosition{
        .x = 110.0,
        .y = 190.0,
    };

    constexpr Platform::CursorPosition kThirdCursorPosition{
        .x = 125.0,
        .y = 195.0,
    };

    constexpr double kExpectedTotalCursorDeltaX = 25.0;
    constexpr double kExpectedTotalCursorDeltaY = -5.0;

    constexpr double kExpectedLastCursorDeltaX = 15.0;
    constexpr double kExpectedLastCursorDeltaY = 5.0;

    auto deliverEvent(Platform::InputState &state,
                      const Platform::PlatformEvent &event) -> void {
        state.applyEvents(std::span<const Platform::PlatformEvent>{&event, 1U});
    }
} // namespace

TEST(InputStateTests, StartsWithAllInputsReleased) {
    Platform::InputState state;

    constexpr std::array keys{
        Platform::Key::W,         Platform::Key::S,   Platform::Key::A,
        Platform::Key::D,         Platform::Key::Q,   Platform::Key::E,
        Platform::Key::LeftShift, Platform::Key::Tab,
    };

    constexpr std::array buttons{
        Platform::MouseButton::Left,
        Platform::MouseButton::Right,
        Platform::MouseButton::Middle,
    };

    for (const Platform::Key key : keys) {
        EXPECT_FALSE(state.isKeyDown(key));
        EXPECT_FALSE(state.wasKeyPressed(key));
        EXPECT_FALSE(state.wasKeyReleased(key));
    }

    for (const Platform::MouseButton button : buttons) {
        EXPECT_FALSE(state.isMouseButtonDown(button));
        EXPECT_FALSE(state.wasMouseButtonPressed(button));
        EXPECT_FALSE(state.wasMouseButtonReleased(button));
    }
}

TEST(InputStateTests, KeyHeldStatePersistsAcrossFrames) {
    Platform::InputState state;

    deliverEvent(state, Platform::KeyEvent{
                            .key = Platform::Key::W,
                            .action = Platform::KeyAction::Press,
                        });

    EXPECT_TRUE(state.isKeyDown(Platform::Key::W));
    EXPECT_TRUE(state.wasKeyPressed(Platform::Key::W));
    EXPECT_FALSE(state.wasKeyReleased(Platform::Key::W));

    state.beginFrame();

    EXPECT_TRUE(state.isKeyDown(Platform::Key::W));
    EXPECT_FALSE(state.wasKeyPressed(Platform::Key::W));
    EXPECT_FALSE(state.wasKeyReleased(Platform::Key::W));

    deliverEvent(state, Platform::KeyEvent{
                            .key = Platform::Key::W,
                            .action = Platform::KeyAction::Release,
                        });

    EXPECT_FALSE(state.isKeyDown(Platform::Key::W));
    EXPECT_FALSE(state.wasKeyPressed(Platform::Key::W));
    EXPECT_TRUE(state.wasKeyReleased(Platform::Key::W));

    state.beginFrame();

    EXPECT_FALSE(state.isKeyDown(Platform::Key::W));
    EXPECT_FALSE(state.wasKeyReleased(Platform::Key::W));
}

TEST(InputStateTests, KeyRepeatDoesNotGeneratePressTransition) {
    Platform::InputState state;

    deliverEvent(state, Platform::KeyEvent{
                            .key = Platform::Key::W,
                            .action = Platform::KeyAction::Press,
                        });

    state.beginFrame();

    deliverEvent(state, Platform::KeyEvent{
                            .key = Platform::Key::W,
                            .action = Platform::KeyAction::Repeat,
                        });

    EXPECT_TRUE(state.isKeyDown(Platform::Key::W));
    EXPECT_FALSE(state.wasKeyPressed(Platform::Key::W));
    EXPECT_FALSE(state.wasKeyReleased(Platform::Key::W));
}

TEST(InputStateTests, RepeatWithoutPriorPressMarksKeyHeld) {
    Platform::InputState state;

    deliverEvent(state, Platform::KeyEvent{
                            .key = Platform::Key::A,
                            .action = Platform::KeyAction::Repeat,
                        });

    EXPECT_TRUE(state.isKeyDown(Platform::Key::A));
    EXPECT_FALSE(state.wasKeyPressed(Platform::Key::A));
}

TEST(InputStateTests, DuplicateKeyEventsDoNotInventTransitions) {
    Platform::InputState state;

    const Platform::KeyEvent press{
        .key = Platform::Key::S,
        .action = Platform::KeyAction::Press,
    };

    const Platform::KeyEvent release{
        .key = Platform::Key::S,
        .action = Platform::KeyAction::Release,
    };

    deliverEvent(state, press);
    state.beginFrame();

    deliverEvent(state, press);

    EXPECT_TRUE(state.isKeyDown(Platform::Key::S));
    EXPECT_FALSE(state.wasKeyPressed(Platform::Key::S));

    deliverEvent(state, release);
    state.beginFrame();

    deliverEvent(state, release);

    EXPECT_FALSE(state.isKeyDown(Platform::Key::S));
    EXPECT_FALSE(state.wasKeyReleased(Platform::Key::S));
}

TEST(InputStateTests, PreservesPressAndReleaseWithinSameFrame) {
    Platform::InputState state;

    constexpr std::array<Platform::PlatformEvent, 2> events{
        Platform::KeyEvent{
            .key = Platform::Key::D,
            .action = Platform::KeyAction::Press,
        },
        Platform::KeyEvent{
            .key = Platform::Key::D,
            .action = Platform::KeyAction::Release,
        },
    };

    state.applyEvents(events);

    EXPECT_FALSE(state.isKeyDown(Platform::Key::D));
    EXPECT_TRUE(state.wasKeyPressed(Platform::Key::D));
    EXPECT_TRUE(state.wasKeyReleased(Platform::Key::D));

    state.beginFrame();

    EXPECT_FALSE(state.wasKeyPressed(Platform::Key::D));
    EXPECT_FALSE(state.wasKeyReleased(Platform::Key::D));
}

TEST(InputStateTests, MouseButtonHeldStatePersistsAcrossFrames) {
    Platform::InputState state;

    deliverEvent(state, Platform::MouseButtonEvent{
                            .button = Platform::MouseButton::Left,
                            .action = Platform::MouseButtonAction::Press,
                        });

    EXPECT_TRUE(state.isMouseButtonDown(Platform::MouseButton::Left));
    EXPECT_TRUE(state.wasMouseButtonPressed(Platform::MouseButton::Left));

    state.beginFrame();

    EXPECT_TRUE(state.isMouseButtonDown(Platform::MouseButton::Left));
    EXPECT_FALSE(state.wasMouseButtonPressed(Platform::MouseButton::Left));

    deliverEvent(state, Platform::MouseButtonEvent{
                            .button = Platform::MouseButton::Left,
                            .action = Platform::MouseButtonAction::Release,
                        });

    EXPECT_FALSE(state.isMouseButtonDown(Platform::MouseButton::Left));
    EXPECT_TRUE(state.wasMouseButtonReleased(Platform::MouseButton::Left));
}

TEST(InputStateTests, MousePressAndReleaseInSameFramePreserveTransitions) {
    Platform::InputState state;

    constexpr std::array<Platform::PlatformEvent, 2> events{
        Platform::MouseButtonEvent{
            .button = Platform::MouseButton::Right,
            .action = Platform::MouseButtonAction::Press,
        },
        Platform::MouseButtonEvent{
            .button = Platform::MouseButton::Right,
            .action = Platform::MouseButtonAction::Release,
        },
    };

    state.applyEvents(events);

    EXPECT_FALSE(state.isMouseButtonDown(Platform::MouseButton::Right));
    EXPECT_TRUE(state.wasMouseButtonPressed(Platform::MouseButton::Right));
    EXPECT_TRUE(state.wasMouseButtonReleased(Platform::MouseButton::Right));

    state.beginFrame();

    EXPECT_FALSE(state.wasMouseButtonPressed(Platform::MouseButton::Right));
    EXPECT_FALSE(state.wasMouseButtonReleased(Platform::MouseButton::Right));
}

TEST(InputStateTests, LosingFocusClearsKeyboardAndMouseStates) {
    Platform::InputState state;

    deliverEvent(state, Platform::KeyEvent{
                            .key = Platform::Key::W,
                            .action = Platform::KeyAction::Press,
                        });

    deliverEvent(state, Platform::MouseButtonEvent{
                            .button = Platform::MouseButton::Middle,
                            .action = Platform::MouseButtonAction::Press,
                        });

    deliverEvent(state, Platform::WindowFocusEvent{.focused = false});

    EXPECT_FALSE(state.isKeyDown(Platform::Key::W));
    EXPECT_FALSE(state.wasKeyPressed(Platform::Key::W));
    EXPECT_FALSE(state.wasKeyReleased(Platform::Key::W));

    EXPECT_FALSE(state.isMouseButtonDown(Platform::MouseButton::Middle));
    EXPECT_FALSE(state.wasMouseButtonPressed(Platform::MouseButton::Middle));
    EXPECT_FALSE(state.wasMouseButtonReleased(Platform::MouseButton::Middle));
}

TEST(InputStateTests, GainingFocusDoesNotInventInput) {
    Platform::InputState state;

    deliverEvent(state, Platform::WindowFocusEvent{.focused = true});

    EXPECT_FALSE(state.isKeyDown(Platform::Key::W));
    EXPECT_FALSE(state.wasKeyPressed(Platform::Key::W));

    deliverEvent(state, Platform::KeyEvent{
                            .key = Platform::Key::W,
                            .action = Platform::KeyAction::Press,
                        });

    deliverEvent(state, Platform::WindowFocusEvent{.focused = true});

    EXPECT_TRUE(state.isKeyDown(Platform::Key::W));
    EXPECT_TRUE(state.wasKeyPressed(Platform::Key::W));
}

TEST(InputStateTests, PreservesEventOrderAroundFocusLoss) {
    Platform::InputState state;

    constexpr std::array<Platform::PlatformEvent, 3> events{
        Platform::KeyEvent{
            .key = Platform::Key::W,
            .action = Platform::KeyAction::Press,
        },
        Platform::WindowFocusEvent{.focused = false},
        Platform::KeyEvent{
            .key = Platform::Key::A,
            .action = Platform::KeyAction::Press,
        },
    };

    state.applyEvents(events);

    EXPECT_FALSE(state.isKeyDown(Platform::Key::W));
    EXPECT_FALSE(state.wasKeyPressed(Platform::Key::W));

    EXPECT_TRUE(state.isKeyDown(Platform::Key::A));
    EXPECT_TRUE(state.wasKeyPressed(Platform::Key::A));
}

TEST(InputStateTests, IgnoresUnrelatedPlatformEvents) {
    Platform::InputState state;

    deliverEvent(state, Platform::KeyEvent{
                            .key = Platform::Key::W,
                            .action = Platform::KeyAction::Press,
                        });

    constexpr std::array<Platform::PlatformEvent, 3> events{
        Platform::WindowResizedEvent{
            .width = 800,
            .height = 600,
        },
        Platform::FramebufferResizedEvent{
            .width = 1600,
            .height = 1200,
        },
        Platform::WindowCloseRequestedEvent{},
    };

    state.applyEvents(events);

    EXPECT_TRUE(state.isKeyDown(Platform::Key::W));
    EXPECT_TRUE(state.wasKeyPressed(Platform::Key::W));
}

TEST(InputStateTests, InputStateInstancesAreIndependent) {
    Platform::InputState first;
    Platform::InputState second;

    deliverEvent(first, Platform::KeyEvent{
                            .key = Platform::Key::W,
                            .action = Platform::KeyAction::Press,
                        });

    EXPECT_TRUE(first.isKeyDown(Platform::Key::W));
    EXPECT_FALSE(second.isKeyDown(Platform::Key::W));
}

TEST(InputStateTests, InvalidKeyQueriesFailFast) {
    Platform::InputState state;
    const auto invalid_key = std::bit_cast<Platform::Key>(std::uint8_t{255});

    EXPECT_DEATH((void)state.isKeyDown(invalid_key),
                 "InputState requires a supported Key value");
    EXPECT_DEATH((void)state.wasKeyPressed(invalid_key),
                 "InputState requires a supported Key value");
    EXPECT_DEATH((void)state.wasKeyReleased(invalid_key),
                 "InputState requires a supported Key value");
}

TEST(InputStateTests, InvalidMouseButtonQueriesFailFast) {
    Platform::InputState state;
    const auto invalid_button =
        std::bit_cast<Platform::MouseButton>(std::uint8_t{255});

    EXPECT_DEATH((void)state.isMouseButtonDown(invalid_button),
                 "InputState requires a supported MouseButton value");
    EXPECT_DEATH((void)state.wasMouseButtonPressed(invalid_button),
                 "InputState requires a supported MouseButton value");
    EXPECT_DEATH((void)state.wasMouseButtonReleased(invalid_button),
                 "InputState requires a supported MouseButton value");
}

TEST(InputStateTests, InvalidInputEventsFailFast) {
    Platform::InputState state;

    const auto invalid_key = std::bit_cast<Platform::Key>(std::uint8_t{255});

    const auto invalid_button =
        std::bit_cast<Platform::MouseButton>(std::uint8_t{255});

    const Platform::PlatformEvent invalid_key_event = Platform::KeyEvent{
        .key = invalid_key,
        .action = Platform::KeyAction::Press,
    };

    const Platform::PlatformEvent invalid_button_event =
        Platform::MouseButtonEvent{
            .button = invalid_button,
            .action = Platform::MouseButtonAction::Press,
    };

    EXPECT_DEATH(deliverEvent(state, invalid_key_event),
                 "InputState requires a supported Key value");

    EXPECT_DEATH(deliverEvent(state, invalid_button_event),
                 "InputState requires a supported MouseButton value");
}

TEST(InputStateTests, StartsWithZeroScrollDelta) {
    Platform::InputState state;

    const auto delta = state.scrollDelta();

    EXPECT_DOUBLE_EQ(delta.x, kZeroScroll);
    EXPECT_DOUBLE_EQ(delta.y, kZeroScroll);
}

TEST(InputStateTests, TracksVerticalOnlyScroll) {
    Platform::InputState state;

    deliverEvent(state, Platform::ScrollEvent{
                            .x_offset = kZeroScroll,
                            .y_offset = kOneAndQuarterScroll,
                        });

    const auto delta = state.scrollDelta();

    EXPECT_DOUBLE_EQ(delta.x, kZeroScroll);
    EXPECT_DOUBLE_EQ(delta.y, kOneAndQuarterScroll);
}

TEST(InputStateTests, TracksHorizontalOnlyScroll) {
    Platform::InputState state;

    deliverEvent(state, Platform::ScrollEvent{
                            .x_offset = -kThreeQuarterScroll,
                            .y_offset = kZeroScroll,
                        });

    const auto delta = state.scrollDelta();

    EXPECT_DOUBLE_EQ(delta.x, -kThreeQuarterScroll);
    EXPECT_DOUBLE_EQ(delta.y, kZeroScroll);
}

TEST(InputStateTests, AccumulatesMultipleScrollEventsWithinFrame) {
    Platform::InputState state;

    constexpr std::array<Platform::PlatformEvent, 3> events{
        Platform::ScrollEvent{
            .x_offset = kZeroScroll,
            .y_offset = kOneScroll,
        },
        Platform::ScrollEvent{
            .x_offset = kHalfScroll,
            .y_offset = kQuarterScroll,
        },
        Platform::ScrollEvent{
            .x_offset = -kQuarterScroll,
            .y_offset = -kHalfScroll,
        },
    };

    state.applyEvents(events);

    const auto delta = state.scrollDelta();

    EXPECT_DOUBLE_EQ(delta.x, kQuarterScroll);
    EXPECT_DOUBLE_EQ(delta.y, kThreeQuarterScroll);
}

TEST(InputStateTests, BeginFrameClearsScrollButPreservesHeldKeys) {
    Platform::InputState state;

    deliverEvent(state, Platform::KeyEvent{
                            .key = Platform::Key::W,
                            .action = Platform::KeyAction::Press,
                        });

    deliverEvent(state, Platform::ScrollEvent{
                            .x_offset = kHalfScroll,
                            .y_offset = kTwoScroll,
                        });

    state.beginFrame();

    const auto delta = state.scrollDelta();

    EXPECT_DOUBLE_EQ(delta.x, kZeroScroll);
    EXPECT_DOUBLE_EQ(delta.y, kZeroScroll);

    EXPECT_TRUE(state.isKeyDown(Platform::Key::W));
    EXPECT_FALSE(state.wasKeyPressed(Platform::Key::W));

    deliverEvent(state, Platform::ScrollEvent{
                            .x_offset = kZeroScroll,
                            .y_offset = -kOneScroll,
                        });

    EXPECT_DOUBLE_EQ(state.scrollDelta().y, -kOneScroll);
}

TEST(InputStateTests, FocusLossClearsAccumulatedScroll) {
    Platform::InputState state;

    deliverEvent(state, Platform::ScrollEvent{
                            .x_offset = kOneScroll,
                            .y_offset = kTwoScroll,
                        });

    deliverEvent(state, Platform::WindowFocusEvent{
                            .focused = false,
                        });

    const auto delta = state.scrollDelta();

    EXPECT_DOUBLE_EQ(delta.x, kZeroScroll);
    EXPECT_DOUBLE_EQ(delta.y, kZeroScroll);
}

TEST(InputStateTests, PreservesScrollEventOrderAroundFocusLoss) {
    Platform::InputState state;

    constexpr std::array<Platform::PlatformEvent, 3> events{
        Platform::ScrollEvent{
            .x_offset = kOneScroll,
            .y_offset = kTwoScroll,
        },
        Platform::WindowFocusEvent{
            .focused = false,
        },
        Platform::ScrollEvent{
            .x_offset = -kQuarterScroll,
            .y_offset = kHalfScroll,
        },
    };

    state.applyEvents(events);

    const auto delta = state.scrollDelta();

    EXPECT_DOUBLE_EQ(delta.x, -kQuarterScroll);
    EXPECT_DOUBLE_EQ(delta.y, kHalfScroll);
}

TEST(InputStateTests, FirstCursorEventEstablishesPositionWithoutMovement) {
    Platform::InputState state;

    EXPECT_FALSE(state.cursorPosition().has_value());
    EXPECT_DOUBLE_EQ(state.cursorDelta().x, kCursorOrigin.x);
    EXPECT_DOUBLE_EQ(state.cursorDelta().y, kCursorOrigin.y);

    deliverEvent(state, Platform::CursorMovedEvent{
                            .position = kInitialCursorPosition,
                        });

    const auto position = state.cursorPosition();
    ASSERT_TRUE(position.has_value());

    const Platform::CursorPosition actual_position =
        position.value_or(kCursorOrigin);

    EXPECT_DOUBLE_EQ(actual_position.x, kInitialCursorPosition.x);
    EXPECT_DOUBLE_EQ(actual_position.y, kInitialCursorPosition.y);

    EXPECT_DOUBLE_EQ(state.cursorDelta().x, kCursorOrigin.x);
    EXPECT_DOUBLE_EQ(state.cursorDelta().y, kCursorOrigin.y);
}

TEST(InputStateTests, AccumulatesCursorMovementWithinFrame) {
    Platform::InputState state;

    constexpr std::array<Platform::PlatformEvent, 3> events{
        Platform::CursorMovedEvent{
            .position = kInitialCursorPosition,
        },
        Platform::CursorMovedEvent{
            .position = kSecondCursorPosition,
        },
        Platform::CursorMovedEvent{
            .position = kThirdCursorPosition,
        },
    };

    state.applyEvents(events);

    const auto position = state.cursorPosition();
    ASSERT_TRUE(position.has_value());

    const Platform::CursorPosition actual_position =
        position.value_or(kCursorOrigin);

    EXPECT_DOUBLE_EQ(actual_position.x, kThirdCursorPosition.x);
    EXPECT_DOUBLE_EQ(actual_position.y, kThirdCursorPosition.y);

    const auto delta = state.cursorDelta();

    EXPECT_DOUBLE_EQ(delta.x, kExpectedTotalCursorDeltaX);
    EXPECT_DOUBLE_EQ(delta.y, kExpectedTotalCursorDeltaY);
}

TEST(InputStateTests, AcceptsCursorMovementToOrigin) {
    Platform::InputState state;

    deliverEvent(state, Platform::CursorMovedEvent{
                            .position = kInitialCursorPosition,
                        });

    deliverEvent(state, Platform::CursorMovedEvent{
                            .position = kCursorOrigin,
                        });

    const auto position = state.cursorPosition();
    ASSERT_TRUE(position.has_value());

    const Platform::CursorPosition actual_position =
        position.value_or(kCursorOrigin);

    EXPECT_DOUBLE_EQ(actual_position.x, kCursorOrigin.x);
    EXPECT_DOUBLE_EQ(actual_position.y, kCursorOrigin.y);

    const auto delta = state.cursorDelta();

    EXPECT_DOUBLE_EQ(delta.x, -kInitialCursorPosition.x);
    EXPECT_DOUBLE_EQ(delta.y, -kInitialCursorPosition.y);
}

TEST(InputStateTests, BeginFramePreservesPositionButClearsCursorDelta) {
    Platform::InputState state;

    deliverEvent(state, Platform::CursorMovedEvent{
                            .position = kInitialCursorPosition,
                        });

    deliverEvent(state, Platform::CursorMovedEvent{
                            .position = kSecondCursorPosition,
                        });

    state.beginFrame();

    const auto position = state.cursorPosition();
    ASSERT_TRUE(position.has_value());

    const Platform::CursorPosition actual_position =
        position.value_or(kCursorOrigin);

    EXPECT_DOUBLE_EQ(actual_position.x, kSecondCursorPosition.x);
    EXPECT_DOUBLE_EQ(actual_position.y, kSecondCursorPosition.y);

    EXPECT_DOUBLE_EQ(state.cursorDelta().x, kCursorOrigin.x);
    EXPECT_DOUBLE_EQ(state.cursorDelta().y, kCursorOrigin.y);

    deliverEvent(state, Platform::CursorMovedEvent{
                            .position = kThirdCursorPosition,
                        });

    const auto delta = state.cursorDelta();

    EXPECT_DOUBLE_EQ(delta.x, kExpectedLastCursorDeltaX);
    EXPECT_DOUBLE_EQ(delta.y, kExpectedLastCursorDeltaY);
}

TEST(InputStateTests, FocusLossInvalidatesCursorAndRebasesMovement) {
    Platform::InputState state;

    deliverEvent(state, Platform::CursorMovedEvent{
                            .position = kInitialCursorPosition,
                        });

    deliverEvent(state, Platform::CursorMovedEvent{
                            .position = kSecondCursorPosition,
                        });

    deliverEvent(state, Platform::WindowFocusEvent{
                            .focused = false,
                        });

    EXPECT_FALSE(state.cursorPosition().has_value());
    EXPECT_DOUBLE_EQ(state.cursorDelta().x, kCursorOrigin.x);
    EXPECT_DOUBLE_EQ(state.cursorDelta().y, kCursorOrigin.y);

    deliverEvent(state, Platform::WindowFocusEvent{
                            .focused = true,
                        });

    deliverEvent(state, Platform::CursorMovedEvent{
                            .position = kSecondCursorPosition,
                        });

    EXPECT_TRUE(state.cursorPosition().has_value());
    EXPECT_DOUBLE_EQ(state.cursorDelta().x, kCursorOrigin.x);
    EXPECT_DOUBLE_EQ(state.cursorDelta().y, kCursorOrigin.y);

    deliverEvent(state, Platform::CursorMovedEvent{
                            .position = kThirdCursorPosition,
                        });

    const auto delta = state.cursorDelta();

    EXPECT_DOUBLE_EQ(delta.x, kExpectedLastCursorDeltaX);
    EXPECT_DOUBLE_EQ(delta.y, kExpectedLastCursorDeltaY);
}
