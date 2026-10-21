#pragma once

#include "engine/renderer/resource/gpu_resource_handle.hpp"

namespace SNE::Engine::Renderer {
    /**
     * @brief Describes a dependency between two GPU resources.
     *
     * Identifies a resource that depends on another resource using
     * their registry-scoped, generation-checked handles.
     *
     * The relationship is directional: dependent requires dependency.
     * Reversing the handles describes a different relationship.
     *
     * This structure does not own resources, validate handles, track
     * resource lifetimes, or synchronize GPU execution.
     */
    struct GpuResourceDependency {
        /**
         * @brief Handle of the resource that requires another resource.
         */
        GpuResourceHandle dependent{};

        /**
         * @brief Handle of the resource required by the dependent.
         */
        GpuResourceHandle dependency{};

        /**
         * @brief Compares two resource dependency relationships.
         *
         * @param other Relationship to compare.
         * @return True when both resource handles match.
         */
        [[nodiscard]] constexpr auto
        operator==(const GpuResourceDependency &other) const noexcept
            -> bool = default;
    };
} // namespace SNE::Engine::Renderer
