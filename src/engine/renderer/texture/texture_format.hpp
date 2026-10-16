#pragma once

#include <cstdint>
namespace SNE::Engine::Renderer {
    /**
     * @brief Describes the pixel format of a texture.
     *
     * Defines backend-independent texture formats used to interpret
     * stored pixel data.
     *
     * Rendering backends translate these values into native formats.
     */
    enum class TextureFormat : std::uint8_t {
        /** @brief Four normalized 8-bit channels without sRGB conversion. */
        Rgba8Unorm,

        /**
         * @brief Four 8-bit channels with sRGB color interpretation.
         *
         * RGB channels are converted from sRGB to linear when sampled.
         * The alpha channel remains linear.
         */
        Rgba8Srgb,
    };
} // namespace SNE::Engine::Renderer
