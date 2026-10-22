#pragma once

#include <cstdint>

namespace SNE::Engine::Renderer {
    /**
     * @brief Specifies how texels are filtered when sampling a texture.
     *
     * Controls how texture samples are calculated when texture
     * coordinates fall between texel centers.
     *
     * This enumeration is independent of the rendering backend.
     * Each backend translates the selected filtering mode into
     * its corresponding native API representation.
     */
    enum class TextureFilter : std::uint8_t {
        /**
         * @brief Selects the nearest texel without interpolation.
         *
         * Preserves hard texel boundaries and is suitable for
         * pixel-art textures.
         */
        Nearest,

        /**
         * @brief Interpolates between neighboring texels.
         *
         * Produces smoother texture sampling and is suitable
         * for photographic and continuously varying textures.
         */
        Linear,
    };
} // namespace SNE::Engine::Renderer
