#pragma once

#include <cstdint>

namespace SNE::Engine::Renderer {
    /**
     * @brief Allocates a unique GPU resource registry identity.
     *
     * Returns a nonzero identity that is not reused during the process
     * lifetime. Allocation is safe across multiple threads and resource
     * registry types.
     *
     * @return Unique, nonzero registry identity.
     */
    [[nodiscard]] auto allocateGpuResourceRegistryId() -> std::uint64_t;
} // namespace SNE::Engine::Renderer
