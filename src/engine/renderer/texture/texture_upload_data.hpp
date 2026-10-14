#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace SNE::Engine::Renderer {
    /**
     * @brief Describes CPU-side pixel data for a texture upload.
     *
     * Contains the dimensions and tightly packed RGBA8 pixel bytes
     * required to initialize a texture.
     *
     * Each pixel occupies four bytes in red, green, blue, and alpha
     * order. The required storage size is width * height * 4 bytes.
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
         * @brief Texture width in pixels.
         */
        std::uint32_t width{};

        /**
         * @brief Texture height in pixels.
         */
        std::uint32_t height{};

        /**
         * @brief Borrowed, tightly packed RGBA8 pixel data.
         *
         * The span must reference width * height * 4 bytes
         * when used for a texture upload.
         *
         * The referenced memory must remain valid throughout
         * the consuming upload operation.
         */
        std::span<const std::byte> pixels;
    };
} // namespace SNE::Engine::Renderer
