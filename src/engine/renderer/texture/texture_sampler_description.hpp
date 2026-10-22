#pragma once

#include "engine/renderer/texture/texture_address_mode.hpp"
#include "engine/renderer/texture/texture_filter.hpp"

namespace SNE::Engine::Renderer {
    /**
     * @brief Describes texture sampling behavior independently of the graphics
     * API.
     *
     * Specifies the filtering methods used during texture magnification
     * and minification, together with the addressing modes applied to
     * texture coordinates along the U, V, and W axes.
     *
     * Rendering backends translate these settings into their corresponding
     * native sampler configuration.
     *
     * By default, magnification and minification use linear filtering,
     * while all texture coordinate axes use repeat addressing.
     *
     * This structure describes sampler configuration only. It does not
     * own texture data, GPU resources, or native sampler handles.
     */
    struct TextureSamplerDescription {
        /**
         * @brief Filtering method used when magnifying a texture.
         *
         * Determines how texels are sampled when they cover multiple
         * screen pixels.
         *
         * Defaults to linear filtering.
         */
        TextureFilter magnification_filter{TextureFilter::Linear};

        /**
         * @brief Filtering method used when minifying a texture.
         *
         * Determines how texels are sampled when multiple texels
         * contribute to a screen pixel.
         *
         * Defaults to linear filtering.
         */
        TextureFilter minification_filter{TextureFilter::Linear};

        /**
         * @brief Texture addressing mode along the U coordinate axis.
         *
         * Determines how horizontal texture coordinates outside the
         * normalized range are handled.
         *
         * Defaults to repeating the texture.
         */
        TextureAddressMode address_mode_u{TextureAddressMode::Repeat};

        /**
         * @brief Texture addressing mode along the V coordinate axis.
         *
         * Determines how vertical texture coordinates outside the
         * normalized range are handled.
         *
         * Defaults to repeating the texture.
         */
        TextureAddressMode address_mode_v{TextureAddressMode::Repeat};

        /**
         * @brief Texture addressing mode along the W coordinate axis.
         *
         * Determines how coordinates along the third texture axis
         * outside the normalized range are handled.
         *
         * Defaults to repeating the texture.
         */
        TextureAddressMode address_mode_w{TextureAddressMode::Repeat};
    };
} // namespace SNE::Engine::Renderer
