#pragma once

#include <cstdint>
namespace SNE::Engine::Renderer {
    /**
     * @brief Describes the intended CPU/GPU access pattern for a GPU
     * allocation.
     *
     * The selected usage expresses engine-level memory intent rather than
     * backend-specific memory properties.
     */
    enum class GpuMemoryUsage : std::uint8_t {
        /** Optimized for GPU access; CPU access is not required. */
        Device,
        /** Optimized for CPU writes that will be consumed by the GPU. */
        Upload,
        /** Optimized for CPU reads of data produced by the GPU. */
        Readback,
    };
} // namespace SNE::Engine::Renderer
