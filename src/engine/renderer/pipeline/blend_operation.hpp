#pragma once

#include <cstdint>

namespace SNE::Engine::Renderer {
    /**
     * @brief Specifies how scaled source and destination values are combined.
     *
     * Defines the operation used to calculate the resulting color
     * or alpha value during blending.
     */
    enum class BlendOperation : std::uint8_t {
        /** @brief Adds the scaled source and destination values. */
        Add,

        /** @brief Subtracts the scaled destination from the scaled source. */
        Subtract,

        /** @brief Subtracts the scaled source from the scaled destination. */
        ReverseSubtract,

        /** @brief Selects the minimum of the source and destination values. */
        Min,

        /** @brief Selects the maximum of the source and destination values. */
        Max,
    };
} // namespace SNE::Engine::Renderer
