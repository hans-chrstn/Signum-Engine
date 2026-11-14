#pragma once

#include <cstdint>

namespace SNE::Engine::Renderer {
    /**
     * @brief Identifies the graphics API selected for rendering.
     *
     * Represents the backend implementation used to create graphics
     * devices and execute rendering operations.
     *
     * Does not identify an individual physical or logical device.
     *
     * @note Vulkan is currently the only implemented graphics backend.
     */
    enum class GraphicsBackend : std::uint8_t {
        /** Uses the Vulkan graphics API. */
        Vulkan,
    };
} // namespace SNE::Engine::Renderer
