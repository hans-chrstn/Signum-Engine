#pragma once

namespace SNE::Engine::Renderer {
    /**
     * @brief Specifies which color attachment components may be written.
     *
     * Each enabled component permits writes to its corresponding channel.
     * All components are enabled by default.
     */
    struct ColorWriteMask {
        /** @brief Enables writes to the red component. */
        bool red{true};

        /** @brief Enables writes to the green component. */
        bool green{true};

        /** @brief Enables writes to the blue component. */
        bool blue{true};

        /** @brief Enables writes to the alpha component. */
        bool alpha{true};

        /**
         * @brief Compares the enabled color write channels.
         *
         * @param other Color write mask to compare.
         *
         * @return True when all channel settings match.
         */
        [[nodiscard]] constexpr auto
        operator==(const ColorWriteMask &other) const noexcept
            -> bool = default;
    };
} // namespace SNE::Engine::Renderer
