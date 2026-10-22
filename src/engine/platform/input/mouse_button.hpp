#pragma once

#include <cstdint>

namespace SNE::Engine::Platform {
    /**
     * @brief Identifies a mouse button independently of the windowing backend.
     *
     * Provides backend-independent mouse button identifiers for
     * querying mouse input through the platform window interface.
     *
     * Platform implementations translate these values into their
     * corresponding native mouse button identifiers.
     */
    enum class MouseButton : std::uint8_t {
        /** @brief Primary mouse button, typically the left button. */
        Left,

        /** @brief Secondary mouse button, typically the right button. */
        Right,

        /** @brief Middle mouse button, typically the scroll-wheel button. */
        Middle,
    };
} // namespace SNE::Engine::Platform
