
#pragma once

#include "engine/renderer/texture/rgba8_image.hpp"
#include <filesystem>

namespace SNE::Engine::Renderer {
    /**
     * @brief Loads and decodes an image file into owned RGBA8 pixel data.
     *
     * Reads an encoded image from the specified filesystem path and
     * converts its pixels into tightly packed RGBA8 format.
     *
     * The returned image owns its decoded pixel storage and remains
     * valid independently of the original file and decoder.
     *
     * The operation is CPU-side and does not create GPU resources.
     * Color-space interpretation is left to the consuming texture
     * configuration.
     *
     * @param path Filesystem path of the image to load.
     *
     * @return Decoded image containing its dimensions and owned
     * RGBA8 pixel storage.
     *
     * @throws Core::Error::EngineError if the image cannot be opened,
     * decoded, or represented in the supported image format.
     * @throws std::bad_alloc if pixel storage allocation fails.
     */
    [[nodiscard]] auto loadRgba8Image(const std::filesystem::path &path)
        -> Rgba8Image;
} // namespace SNE::Engine::Renderer
