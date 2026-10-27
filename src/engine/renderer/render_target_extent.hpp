#pragma once

#include <cstdint>
namespace SNE::Engine::Renderer {
    /**
     * @brief Describes the pixel dimensions of a rendering target.
     *
     * Stores the width and height of a rendering destination
     * independently of the underlying graphics API.
     *
     * The dimensions represent framebuffer pixels rather than
     * logical window or user-interface coordinates.
     *
     * A valid rendering target requires nonzero dimensions.
     * Callers are responsible for validating the extent before
     * requesting rendering resources.
     */
    struct RenderTargetExtent {
        /** @brief Render-target width in pixels. */
        std::uint32_t width{};

        /** @brief Render-target height in pixels. */
        std::uint32_t height{};
    };
} // namespace SNE::Engine::Renderer
