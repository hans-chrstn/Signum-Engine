#pragma once

namespace SNE::Engine::Platform {

    /**
     * @brief Represents a cursor position relative to a window.
     *
     * Stores the cursor's horizontal and vertical coordinates
     * in window-relative coordinate units.
     *
     * The coordinate origin is at the upper-left corner of the
     * window's content area. Positive X extends to the right,
     * while positive Y extends downward.
     *
     * Coordinates are expressed in logical window units rather
     * than framebuffer pixels.
     *
     * This structure is independent of the windowing backend.
     */
    struct CursorPosition {
        /**
         * @brief Horizontal cursor position relative to the window.
         *
         * Increases toward the right.
         */
        double x{};

        /**
         * @brief Vertical cursor position relative to the window.
         *
         * Increases downward.
         */
        double y{};
    };
} // namespace SNE::Engine::Platform
