#pragma once

#include <cstdint>
namespace SNE::Engine::Renderer {
    /**
     * @brief Identifies a GPU resource independently of the rendering backend.
     *
     * Stores the owning registry identity, resource slot index, and
     * generation used to distinguish individual resource lifetimes.
     *
     * This value does not own GPU resources or memory allocations.
     * Resource lookup and lifetime validation belong to the owning registry.
     *
     * A valid resource handle does not independently establish whether
     * GPU-side initialization is complete or whether the required
     * synchronization has been satisfied.
     *
     * Resource readiness belongs to initialization and submission
     * mechanisms, not to the resource handle's identity.
     */
    struct GpuResourceHandle {
        /** @brief Slot index assigned by the owning resource registry. */
        std::uint32_t index{};

        /**
         * @brief Generation identifying the resource occupying the slot.
         *
         * Generation zero is reserved for an invalid handle.
         */
        std::uint32_t generation{};

        /**
         * @brief Identifies the registry that owns this resource.
         *
         * Zero is reserved for an invalid registry identity.
         */
        std::uint64_t registry_id{};

        /**
         * @brief Compares two GPU resource identities.
         *
         * @param other Resource handle to compare.
         * @return True when registry identity, slot index, and generation
         * match.
         */
        [[nodiscard]] constexpr auto
        operator==(const GpuResourceHandle &other) const noexcept
            -> bool = default;
    };
} // namespace SNE::Engine::Renderer
