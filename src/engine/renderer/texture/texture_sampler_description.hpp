#pragma once

#include "engine/renderer/texture/texture_filter.hpp"

namespace SNE::Engine::Renderer {
    /**
     * @brief Describes texture sampling behavior independently of the graphics
     * API.
     *
     * Specifies the filtering methods used when sampling a texture at
     * different levels of magnification or minification.
     *
     * Rendering backends translate these settings into their corresponding
     * native sampler configuration.
     *
     * The default filtering mode is linear for both operations.
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
    };
} // namespace SNE::Engine::Renderer
