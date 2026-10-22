#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace SNE::Engine::Renderer {
    /**
     * @brief Owns decoded RGBA8 image data in CPU memory.
     *
     * Stores image dimensions and tightly packed pixel data using
     * eight bits per red, green, blue, and alpha channel.
     *
     * Each pixel occupies four consecutive bytes in RGBA order.
     * Pixel rows are stored consecutively without padding.
     *
     * The structure owns its pixel storage independently of the
     * original image file and any graphics backend.
     *
     * A default-constructed image has zero dimensions and
     * an empty pixel buffer.
     *
     * Successfully decoded images contain width * height * 4
     * bytes of pixel data.
     *
     * @note Color-space interpretation is determined separately
     * by the consuming texture upload configuration.
     */
    struct Rgba8Image {
        /**
         * @brief Image width in pixels.
         */
        std::uint32_t width{};

        /**
         * @brief Image height in pixels.
         */
        std::uint32_t height{};

        /**
         * @brief Owned, tightly packed RGBA8 pixel data.
         *
         * Contains four bytes per pixel in red, green, blue,
         * and alpha channel order.
         *
         * The vector owns its storage and releases it
         * when the image is destroyed.
         */
        std::vector<std::byte> pixels;
    };
} // namespace SNE::Engine::Renderer
