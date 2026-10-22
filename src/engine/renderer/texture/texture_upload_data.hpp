#pragma once

#include "engine/renderer/texture/texture_format.hpp"
#include "engine/renderer/texture/texture_sampler_description.hpp"
#include <cstddef>
#include <cstdint>
#include <span>

namespace SNE::Engine::Renderer {
    /**
     * @brief Describes CPU-side pixel data for a texture upload.
     *
     * Contains the dimensions, pixel format, mip-level count, sampler
     * configuration, and tightly packed RGBA8 pixel data required
     * to initialize a texture.
     *
     * Each texel occupies four bytes in red, green, blue, and alpha
     * order. Each mip level is stored consecutively, beginning with
     * the full-resolution image and ending with the smallest level.
     *
     * Mip dimensions are halved at each successive level, with each
     * dimension remaining at least one texel.
     *
     * Pixel storage is borrowed rather than owned. The caller must
     * keep the referenced memory valid until the upload operation
     * has finished consuming it.
     *
     * This structure is independent of the rendering backend and
     * does not allocate or own GPU resources.
     */
    struct TextureUploadData {
        /**
         * @brief Texture width in pixels at mip level zero.
         */
        std::uint32_t width{};

        /**
         * @brief Texture height in pixels at mip level zero.
         */
        std::uint32_t height{};

        /**
         * @brief Borrowed, tightly packed RGBA8 pixel data.
         *
         * Contains the complete mip chain without row padding.
         * Each mip level occupies width * height * 4 bytes using
         * that level's dimensions.
         *
         * The referenced memory must remain valid throughout
         * the consuming upload operation.
         */
        std::span<const std::byte> pixels;

        /**
         * @brief Pixel format used to interpret the uploaded texture data.
         *
         * Defaults to RGBA8 UNORM for compatibility with existing callers.
         */
        TextureFormat format{TextureFormat::Rgba8Unorm};

        /**
         * @brief Number of mip levels provided in the pixel data.
         *
         * Defaults to one for textures without additional mip levels.
         *
         * When greater than one, pixels contains the complete mip chain,
         * tightly packed from the largest level to the smallest.
         *
         * @pre Must be greater than zero and must not exceed the
         * maximum mip-level count supported by the texture dimensions.
         */
        std::uint32_t mip_levels{1U};

        /**
         * @brief Sampler configuration used when uploading the texture.
         *
         * Specifies the magnification and minification filtering methods
         * used when sampling the uploaded texture.
         *
         * Defaults to linear filtering for both operations.
         *
         * The configuration is independent of the rendering backend.
         * The renderer translates these settings into its native
         * sampler representation.
         *
         * This member describes sampling behavior and does not
         * own GPU sampler resources.
         */
        TextureSamplerDescription sampler{};
    };
} // namespace SNE::Engine::Renderer
