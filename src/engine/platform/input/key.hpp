#pragma once

#include <cstdint>

namespace SNE::Engine::Platform {
    /**
     * @brief Identifies a keyboard key independently of the platform backend.
     *
     * Provides backend-independent key identifiers for querying keyboard
     * input state.
     *
     * Platform implementations translate these values into their
     * corresponding native key codes.
     */
    enum class Key : std::uint8_t {
        /** @brief W key. */
        W,

        /** @brief S key. */
        S,

        /** @brief A key. */
        A,

        /** @brief D key. */
        D,

        /** @brief Q key. */
        Q,

        /** @brief E key. */
        E,

        /** @brief Left Shift key. */
        LeftShift,

        /** @brief Tab key. */
        Tab,
    };
} // namespace SNE::Engine::Platform
