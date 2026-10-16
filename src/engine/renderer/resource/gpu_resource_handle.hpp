#pragma once

#include <cstdint>
namespace SNE::Engine::Renderer {
    /**
     * @brief Identifies a GPU resource independently of the rendering backend.
     *
     * Stores a resource slot index and generation used to distinguish
     * individual resource lifetimes.
     *
     * This value does not own GPU resources or memory allocations.
     * Resource lookup and lifetime validation belong to the owning registry.
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
         * @brief Compares two GPU resource identities.
         *
         * @param other Resource handle to compare.
         * @return True when both index and generation match.
         */
        [[nodiscard]] constexpr auto
        operator==(const GpuResourceHandle &other) const noexcept
            -> bool = default;
    };
} // namespace SNE::Engine::Renderer
