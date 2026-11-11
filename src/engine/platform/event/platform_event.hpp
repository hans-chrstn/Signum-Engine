#pragma once

#include "engine/platform/input/cursor_position.hpp"
#include "engine/platform/input/key.hpp"
#include "engine/platform/input/mouse_button.hpp"
#include <cstdint>
#include <variant>

namespace SNE::Engine::Platform {
    /**
     * @brief Identifies the action associated with a keyboard event.
     *
     * Distinguishes an initial key press, a key release, and repeated
     * key events generated while a key remains held.
     *
     * This enumeration is independent of platform-specific input codes.
     */
    enum class KeyAction : std::uint8_t {
        /** @brief A key transitioned into the pressed state. */
        Press,

        /** @brief A key transitioned into the released state. */
        Release,

        /** @brief A repeated key event was generated while held. */
        Repeat,
    };

    /**
     * @brief Identifies the action associated with a mouse-button event.
     *
     * Distinguishes a mouse-button press from a release.
     *
     * Unlike keyboard events, mouse-button events do not have a
     * repeat action.
     *
     * This enumeration is independent of platform-specific input codes.
     */
    enum class MouseButtonAction : std::uint8_t {
        /** @brief A mouse button transitioned into the pressed state. */
        Press,

        /** @brief A mouse button transitioned into the released state. */
        Release,
    };

    /**
     * @brief Describes a keyboard input event.
     *
     * Associates a backend-independent key identifier with the action
     * reported for that key.
     *
     * This structure represents an individual event rather than the
     * current held state of the keyboard.
     */
    struct KeyEvent {
        /** @brief Key associated with the event. */
        Key key;

        /** @brief Action reported for the key. */
        KeyAction action;
    };

    /**
     * @brief Describes a mouse-button input event.
     *
     * Associates a backend-independent mouse-button identifier with
     * the action reported for that button.
     *
     * This structure represents an individual event rather than the
     * current held state of a mouse button.
     */
    struct MouseButtonEvent {
        /** @brief Mouse button associated with the event. */
        MouseButton button;

        /** @brief Action reported for the mouse button. */
        MouseButtonAction action;
    };

    /**
     * @brief Describes a cursor movement event.
     *
     * Reports the cursor's updated position relative to the
     * originating window's content area.
     *
     * Coordinates are expressed in logical window units rather
     * than framebuffer pixels.
     *
     * This event stores an absolute cursor position, not the
     * displacement since the previous event.
     */
    struct CursorMovedEvent {
        /**
         * @brief Updated cursor position relative to the window.
         */
        CursorPosition position;
    };

    /**
     * @brief Describes a mouse or pointing-device scroll event.
     *
     * Stores the horizontal and vertical scroll offsets reported
     * by the platform input backend.
     *
     * Offsets represent relative scroll movement rather than
     * absolute positions.
     *
     * Positive and negative values indicate opposite scrolling
     * directions. Fractional offsets are preserved to support
     * high-resolution scrolling devices.
     */
    struct ScrollEvent {
        /** @brief Horizontal scrolling offset. */
        double x_offset{};

        /** @brief Vertical scrolling offset. */
        double y_offset{};
    };

    /**
     * @brief Describes a change to a window's logical dimensions.
     *
     * Reports the updated width and height of the originating
     * window in logical window units.
     *
     * These dimensions may differ from framebuffer dimensions
     * on high-DPI displays.
     */
    struct WindowResizedEvent {
        /** @brief Updated window width in logical units. */
        int width{};

        /** @brief Updated window height in logical units. */
        int height{};
    };

    /**
     * @brief Describes a change to a window's framebuffer dimensions.
     *
     * Reports the updated drawable framebuffer width and height
     * in physical pixels.
     *
     * Framebuffer dimensions are used when updating rendering
     * targets, viewports, and presentation resources.
     *
     * Zero dimensions may occur when a window is minimized.
     */
    struct FramebufferResizedEvent {
        /** @brief Updated framebuffer width in pixels. */
        int width{};

        /** @brief Updated framebuffer height in pixels. */
        int height{};
    };

    /**
     * @brief Describes a change in window input focus.
     *
     * Reports whether the originating window has gained or
     * lost input focus.
     *
     * Focus changes allow input systems to reconcile held
     * input state when a window becomes inactive.
     */
    struct WindowFocusEvent {
        /** @brief True when the window gains focus; false when focus is lost.
         */
        bool focused{false};
    };

    /**
     * @brief Indicates that the platform requested window closure.
     *
     * Represents a close request originating from the window
     * system, such as a user activating the window's close control.
     *
     * This event carries no additional data and does not
     * directly destroy the window or its resources.
     */
    struct WindowCloseRequestedEvent {};

    /**
     * @brief Represents a backend-independent platform event.
     *
     * Stores exactly one supported keyboard, mouse, cursor,
     * scrolling, window-size, focus, or close-request event.
     *
     * Uses a tagged union to preserve the concrete event type
     * without requiring polymorphic allocation or inheritance.
     *
     * Event producers translate native platform notifications
     * into these types before delivering them to engine systems.
     *
     * Consumers may inspect events using std::visit,
     * std::get_if, or std::holds_alternative.
     *
     * Event delivery, buffering, and lifetime management
     * are handled separately from this data representation.
     */
    using PlatformEvent =
        std::variant<KeyEvent, MouseButtonEvent, CursorMovedEvent, ScrollEvent,
                     WindowResizedEvent, FramebufferResizedEvent,
                     WindowFocusEvent, WindowCloseRequestedEvent>;
} // namespace SNE::Engine::Platform
