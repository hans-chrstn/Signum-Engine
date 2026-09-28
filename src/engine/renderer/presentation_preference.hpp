#pragma once

#include <cstdint>

namespace SNE::Engine::Renderer {
    /**
     * @brief Defines the presentation behavior requested from the renderer.
     *
     * Represents backend-independent presentation preferences used by a
     * rendering backend to select an appropriate native presentation mode
     * according to its available capabilities.
     */
    enum class PresentationPreference : std::uint8_t {
        /** Requests synchronized, tear-free presentation. */
        VSync,
        /**
         * Requests synchronized, tear-free presentation with reduced
         * presentation latency when supported.
         */
        LowLatencyVSync,
        /**
         * Requests synchronized presentation that may relax synchronization
         * when the renderer misses the display refresh interval.
         */
        AdaptiveVSync,
        /**
         * Allows unsynchronized presentation in favor of reduced presentation
         * latency.
         */
        AllowTearing,
        /**
         * Requests synchronized presentation that favors the latest ready frame
         * when supported.
         */
        LatestReadyVSync
    };
} // namespace SNE::Engine::Renderer
