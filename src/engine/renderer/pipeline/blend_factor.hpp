#pragma once

#include <cstdint>

namespace SNE::Engine::Renderer {
    /**
     * @brief Specifies a scaling factor used in color blending.
     *
     * Determines how source and destination color or alpha values
     * contribute to a blending operation.
     */
    enum class BlendFactor : std::uint8_t {
        /** @brief Uses a constant factor of zero. */
        Zero,

        /** @brief Uses a constant factor of one. */
        One,

        /** @brief Uses the source color components. */
        SrcColor,

        /** @brief Uses one minus the source color components. */
        OneMinusSrcColor,

        /** @brief Uses the destination color components. */
        DstColor,

        /** @brief Uses one minus the destination color components. */
        OneMinusDstColor,

        /** @brief Uses the source alpha component. */
        SrcAlpha,

        /** @brief Uses one minus the source alpha component. */
        OneMinusSrcAlpha,

        /** @brief Uses the destination alpha component. */
        DstAlpha,

        /** @brief Uses one minus the destination alpha component. */
        OneMinusDstAlpha,
    };
} // namespace SNE::Engine::Renderer
