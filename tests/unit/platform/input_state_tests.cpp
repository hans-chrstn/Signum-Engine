#include "engine/platform/event/platform_event.hpp"
#include "engine/platform/input/input_state.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <gtest/gtest.h>
#include <span>

namespace Platform = SNE::Engine::Platform;

namespace {
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

    constexpr std::array<Platform::PlatformEvent, 5> events{
        Platform::CursorMovedEvent{
            .position = {.x = 100.0, .y = 200.0},
        },
        Platform::ScrollEvent{
            .x_offset = 1.0,
            .y_offset = -2.0,
        },
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
