#pragma once

#include <cstdint>

namespace SNE::Engine::Renderer {
    /**
     * @brief Specifies the intended update frequency of a material buffer.
     *
     * Determines which resource update strategy the renderer should use
     * for a material uniform or storage buffer.
     *
     * Occasional updates replace GPU buffer and descriptor resources,
     * preserving previous resources until outstanding GPU work completes.
     *
     * Frequent updates are intended to use reusable per-frame GPU buffers
     * to avoid repeated resource allocation.
     *
     * @note The frequent-update strategy is not yet implemented.
     */
    enum class MaterialBufferUpdateFrequency : std::uint8_t {
        /** @brief Uses GPU buffer and descriptor resource replacement. */
        Occasional,

        /** @brief Uses reusable per-frame GPU buffer storage when supported. */
        Frequent,
    };
} // namespace SNE::Engine::Renderer
