#include "input_state.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/core/math/vector2d.hpp"
#include "engine/platform/event/platform_event.hpp"
#include <cstddef>
#include <utility>
#include <variant>

namespace SNE::Engine::Platform {
    auto InputState::applyKeyEvent(const KeyEvent &event) noexcept -> void {
        const std::size_t key_index = std::to_underlying(event.key);

        if (key_index >= m_KeyStates.size()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Platform,
                "InputState requires a supported Key value");
        }

        ButtonState &state = m_KeyStates[key_index];

        switch (event.action) {
        case KeyAction::Press:
            if (!state.down) {
                state.pressed = true;
            }

            state.down = true;
            break;
        case KeyAction::Release:
            if (state.down) {
                state.released = true;
            }

            state.down = false;
            break;
        case KeyAction::Repeat:
            state.down = true;
            break;
        }
    }

    auto
    InputState::applyMouseButtonEvent(const MouseButtonEvent &event) noexcept
        -> void {
        const std::size_t button_index = std::to_underlying(event.button);

        if (button_index >= m_MouseButtonStates.size()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Platform,
                "InputState requires a supported MouseButton "
                "value");
        }

        ButtonState &state = m_MouseButtonStates[button_index];

        switch (event.action) {
        case MouseButtonAction::Press:
            if (!state.down) {
                state.pressed = true;
            }

            state.down = true;
            break;
        case MouseButtonAction::Release:
            if (state.down) {
                state.released = true;
            }

            state.down = false;
            break;
        }
    }

    auto InputState::applyScrollEvent(const ScrollEvent &event) noexcept
        -> void {
        if (event.x_offset == 0.0 && event.y_offset == 0.0) {
            return;
        }

        m_ScrollDelta.x += event.x_offset;
        m_ScrollDelta.y += event.y_offset;
    }

    auto
    InputState::applyCursorMovedEvent(const CursorMovedEvent &event) noexcept
        -> void {
        if (m_CursorPosition) {
            m_CursorDelta.x += event.position.x - m_CursorPosition->x;
            m_CursorDelta.y += event.position.y - m_CursorPosition->y;
        }

        m_CursorPosition = event.position;
    }

    auto InputState::clearAll() noexcept -> void {
        m_ScrollDelta = Core::Math::Vector2d{};
        m_CursorDelta = Core::Math::Vector2d{};
        m_CursorPosition.reset();

        for (ButtonState &state : m_KeyStates) {
            state = ButtonState{};
        }

        for (ButtonState &state : m_MouseButtonStates) {
            state = ButtonState{};
        }
    }

    auto InputState::beginFrame() noexcept -> void {
        m_ScrollDelta = Core::Math::Vector2d{};
        m_CursorDelta = Core::Math::Vector2d{};

        for (ButtonState &state : m_KeyStates) {
            state.pressed = false;
            state.released = false;
        }

        for (ButtonState &state : m_MouseButtonStates) {
            state.pressed = false;
            state.released = false;
        }
    }

    auto InputState::applyEvents(std::span<const PlatformEvent> events) noexcept
        -> void {
        for (const PlatformEvent &event : events) {
            if (const auto *key_event = std::get_if<KeyEvent>(&event)) {
                applyKeyEvent(*key_event);
            } else if (const auto *button_event =
                           std::get_if<MouseButtonEvent>(&event)) {
                applyMouseButtonEvent(*button_event);
            } else if (const auto *scroll_event =
                           std::get_if<ScrollEvent>(&event)) {
                applyScrollEvent(*scroll_event);

            } else if (const auto *cursor_event =
                           std::get_if<CursorMovedEvent>(&event)) {
                applyCursorMovedEvent(*cursor_event);

            } else if (const auto *window_event =
                           std::get_if<WindowFocusEvent>(&event);
                       window_event != nullptr && !window_event->focused) {
                clearAll();
            }
        }
    }

    auto InputState::isKeyDown(Key key) const noexcept -> bool {
        const std::size_t key_index = std::to_underlying(key);

        if (key_index >= m_KeyStates.size()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Platform,
                "InputState requires a supported Key value");
        }

        return m_KeyStates[key_index].down;
    }

    auto InputState::wasKeyPressed(Key key) const noexcept -> bool {
        const std::size_t key_index = std::to_underlying(key);

        if (key_index >= m_KeyStates.size()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Platform,
                "InputState requires a supported Key value");
        }

        return m_KeyStates[key_index].pressed;
    }

    auto InputState::wasKeyReleased(Key key) const noexcept -> bool {
        const std::size_t key_index = std::to_underlying(key);

        if (key_index >= m_KeyStates.size()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Platform,
                "InputState requires a supported Key value");
        }

        return m_KeyStates[key_index].released;
    }

    auto InputState::isMouseButtonDown(MouseButton button) const noexcept
        -> bool {
        const std::size_t button_index = std::to_underlying(button);

        if (button_index >= m_MouseButtonStates.size()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Platform,
                "InputState requires a supported MouseButton value");
        }

        return m_MouseButtonStates[button_index].down;
    }

    auto InputState::wasMouseButtonPressed(MouseButton button) const noexcept
        -> bool {
        const std::size_t button_index = std::to_underlying(button);

        if (button_index >= m_MouseButtonStates.size()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Platform,
                "InputState requires a supported MouseButton value");
        }

        return m_MouseButtonStates[button_index].pressed;
    }

    auto InputState::wasMouseButtonReleased(MouseButton button) const noexcept
        -> bool {
        const std::size_t button_index = std::to_underlying(button);

        if (button_index >= m_MouseButtonStates.size()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Platform,
                "InputState requires a supported MouseButton value");
        }

        return m_MouseButtonStates[button_index].released;
    }

    auto InputState::scrollDelta() const noexcept -> Core::Math::Vector2d {
        return m_ScrollDelta;
    }

    auto InputState::cursorPosition() const noexcept
        -> std::optional<CursorPosition> {
        return m_CursorPosition;
    }

    auto InputState::cursorDelta() const noexcept -> Core::Math::Vector2d {
        return m_CursorDelta;
    }
} // namespace SNE::Engine::Platform
