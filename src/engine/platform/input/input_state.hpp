#pragma once

#include "engine/core/math/vector2d.hpp"
#include "engine/platform/event/platform_event.hpp"
#include "engine/platform/input/cursor_position.hpp"
#include "engine/platform/input/key.hpp"
#include "engine/platform/input/mouse_button.hpp"
#include <array>
#include <cstddef>
#include <optional>
#include <span>
#include <utility>

namespace SNE::Engine::Platform {
    /**
     * @brief Tracks backend-independent keyboard and mouse input state.
     *
     * Processes platform events to maintain the current held state
     * of supported keys and mouse buttons, along with their press
     * and release transitions for the current frame.
     *
     * Persistent held states survive frame boundaries, while
     * transient transitions are cleared at the beginning of each frame.
     *
     * Input state is maintained independently of GLFW and does not
     * perform native event polling.
     *
     * @note This class is not thread-safe. Event processing and state
     * queries must be externally synchronized when used across threads.
     */
    class InputState {
      private:
        /**
         * @brief Tracks the held state and frame-local transitions of an input.
         *
         * A button may be both pressed and released within the same frame
         * when multiple transitions occur before the next frame boundary.
         */
        struct ButtonState {
            /**
             * @brief Indicates whether the input is currently held.
             *
             * Persists across frames until the input is released or
             * its state is cleared.
             */
            bool down{false};

            /**
             * @brief Indicates whether a press transition occurred this frame.
             *
             * Remains true until the next input frame begins or
             * the input state is reset.
             */
            bool pressed{false};

            /**
             * @brief Indicates whether a release transition occurred this
             * frame.
             *
             * Remains true until the next input frame begins or
             * the input state is reset.
             */
            bool released{false};
        };

        /**
         * @brief Number of supported keyboard keys.
         *
         * Assumes Key enumerators are contiguous and zero-based,
         * with Key::Tab as the final enumerator.
         */
        static constexpr std::size_t kKeyCount =
            std::to_underlying(Key::Tab) + 1U;

        /**
         * @brief Number of supported mouse buttons.
         *
         * Assumes MouseButton enumerators are contiguous and zero-based,
         * with MouseButton::Middle as the final enumerator.
         */
        static constexpr std::size_t kMouseButtonCount =
            std::to_underlying(MouseButton::Middle) + 1U;

        /**
         * @brief Stores the current state of each supported keyboard key.
         *
         * Entries are indexed by the underlying value of Key.
         */
        std::array<ButtonState, kKeyCount> m_KeyStates{};

        /**
         * @brief Stores the current state of each supported mouse button.
         *
         * Entries are indexed by the underlying value of MouseButton.
         */
        std::array<ButtonState, kMouseButtonCount> m_MouseButtonStates{};

        /**
         * @brief Accumulated horizontal and vertical scroll movement this
         * frame.
         *
         * Reset at frame boundaries and when window focus is lost.
         * Preserves fractional scroll offsets.
         */
        Core::Math::Vector2d m_ScrollDelta{};

        /**
         * @brief Most recently observed cursor position.
         *
         * Remains unavailable until a cursor movement event establishes
         * a position. Invalidated when window focus is lost.
         */
        std::optional<CursorPosition> m_CursorPosition = std::nullopt;

        /**
         * @brief Accumulated cursor displacement during the current frame.
         *
         * Reset at frame boundaries and when window focus is lost.
         * Measured in logical window coordinate units.
         */
        Core::Math::Vector2d m_CursorDelta{};

        /**
         * @brief Applies a keyboard event to the tracked key states.
         *
         * Validates the key identifier and updates its held state and
         * frame-local press or release transitions.
         *
         * @param event Keyboard event to process.
         * @pre The event must identify a supported keyboard key.
         */
        auto applyKeyEvent(const KeyEvent &event) noexcept -> void;

        /**
         * @brief Applies a mouse-button event to the tracked button states.
         *
         * Validates the button identifier and updates its held state
         * and frame-local press or release transitions.
         *
         * @param event Mouse-button event to process.
         * @pre The event must identify a supported mouse button.
         */
        auto applyMouseButtonEvent(const MouseButtonEvent &event) noexcept
            -> void;

        /**
         * @brief Accumulates scrolling movement from a platform event.
         *
         * Adds the event's horizontal and vertical offsets to the
         * current frame's accumulated scroll movement.
         *
         * @param event Scroll event to process.
         */
        auto applyScrollEvent(const ScrollEvent &event) noexcept -> void;

        /**
         * @brief Updates cursor position and accumulated movement.
         *
         * Computes displacement relative to the previously observed
         * cursor position when available, accumulating movement for
         * the current frame.
         *
         * The first cursor event establishes the initial position
         * without generating displacement.
         *
         * @param event Cursor movement event to process.
         */
        auto applyCursorMovedEvent(const CursorMovedEvent &event) noexcept
            -> void;

        /**
         * @brief Clears all tracked keyboard and mouse-button states.
         *
         * Resets held states and frame-local transitions without
         * generating synthetic release events.
         */
        auto clearAll() noexcept -> void;

      public:
        /**
         * @brief Begins a new input frame.
         *
         * Clears all frame-local press and release transitions while
         * preserving the current held state of keys and mouse buttons.
         *
         * @note Call once before processing events for a new input frame.
         */
        auto beginFrame() noexcept -> void;

        /**
         * @brief Applies platform events to the current input state.
         *
         * Processes events in their supplied order, updating keyboard
         * and mouse-button held states and frame-local transitions.
         *
         * Repeated key events preserve the held state without generating
         * additional press transitions.
         *
         * Losing window focus clears all tracked input states to prevent
         * stale held inputs.
         *
         * Events unrelated to input state are ignored.
         *
         * @param events Platform events to process in their original order.
         *
         * @note Does not automatically begin a new input frame.
         */
        auto applyEvents(std::span<const PlatformEvent> events) noexcept
            -> void;

        /**
         * @brief Checks whether a keyboard key is currently held.
         *
         * @param key Keyboard key to query.
         * @return true if the key is held; otherwise false.
         *
         * @pre key must identify a supported keyboard key.
         */
        [[nodiscard]] auto isKeyDown(Key key) const noexcept -> bool;

        /**
         * @brief Checks whether a key was pressed during the current frame.
         *
         * @param key Keyboard key to query.
         * @return true if a press transition occurred during the current
         * frame; otherwise false.
         *
         * @pre key must identify a supported keyboard key.
         */
        [[nodiscard]] auto wasKeyPressed(Key key) const noexcept -> bool;

        /**
         * @brief Checks whether a key was released during the current frame.
         *
         * @param key Keyboard key to query.
         * @return true if a release transition occurred during the current
         * frame; otherwise false.
         *
         * @pre key must identify a supported keyboard key.
         */
        [[nodiscard]] auto wasKeyReleased(Key key) const noexcept -> bool;

        /**
         * @brief Checks whether a mouse button is currently held.
         *
         * @param button Mouse button to query.
         * @return true if the button is held; otherwise false.
         *
         * @pre button must identify a supported mouse button.
         */
        [[nodiscard]] auto isMouseButtonDown(MouseButton button) const noexcept
            -> bool;

        /**
         * @brief Checks whether a mouse button was pressed during the current
         * frame.
         *
         * @param button Mouse button to query.
         * @return true if a press transition occurred during the current
         * frame; otherwise false.
         *
         * @pre button must identify a supported mouse button.
         */
        [[nodiscard]] auto
        wasMouseButtonPressed(MouseButton button) const noexcept -> bool;

        /**
         * @brief Checks whether a mouse button was released during the current
         * frame.
         *
         * @param button Mouse button to query.
         * @return true if a release transition occurred during the current
         * frame; otherwise false.
         *
         * @pre button must identify a supported mouse button.
         */
        [[nodiscard]] auto
        wasMouseButtonReleased(MouseButton button) const noexcept -> bool;

        /**
         * @brief Returns accumulated scroll movement for the current frame.
         *
         * @return Horizontal and vertical scroll offsets accumulated
         * from all processed scroll events during this frame.
         *
         * @note Scroll movement is reset by beginFrame().
         */
        [[nodiscard]] auto scrollDelta() const noexcept -> Core::Math::Vector2d;

        /**
         * @brief Returns the most recently observed cursor position.
         *
         * @return Cursor position in logical window coordinates, or
         * std::nullopt if no valid position has been established.
         */
        [[nodiscard]] auto cursorPosition() const noexcept
            -> std::optional<CursorPosition>;

        /**
         * @brief Returns accumulated cursor movement for the current frame.
         *
         * @return Horizontal and vertical cursor displacement in logical
         * window coordinate units.
         *
         * @note The displacement resets at the beginning of each input frame.
         */
        [[nodiscard]] auto cursorDelta() const noexcept -> Core::Math::Vector2d;
    };
} // namespace SNE::Engine::Platform
