#pragma once

#include <cstdint>

namespace SNE::Engine::Renderer {
    /**
     * @brief Specifies how texture coordinates outside the normalized range
     * are handled during sampling.
     *
     * Defines the addressing behavior used when texture coordinates
     * extend beyond the range from zero to one.
     *
     * The addressing mode can be configured independently for each
     * texture coordinate axis.
     *
     * This enumeration is independent of the rendering backend.
     * Each backend translates these values into its corresponding
     * native sampler addressing modes.
     */
    enum class TextureAddressMode : std::uint8_t {
        /**
         * @brief Repeats the texture outside the normalized coordinate range.
         *
         * Texture coordinates wrap periodically, allowing the texture
         * to tile continuously along the selected axis.
         */
        Repeat,

        /**
         * @brief Repeats the texture with alternating mirrored intervals.
         *
         * Each successive repetition reverses the sampling direction,
         * producing a mirrored tiling pattern.
         */
        MirroredRepeat,

        /**
         * @brief Clamps sampling to the nearest texture edge.
         *
         * Coordinates outside the normalized range sample the
         * nearest edge texels instead of repeating the texture.
         */
        ClampToEdge,
    };
} // namespace SNE::Engine::Renderer
