#pragma once

#include "engine/renderer/material/material_resource_binding.hpp"
#include <vector>

namespace SNE::Engine::Renderer {
    /**
     * @brief Owns a collection of material resource binding descriptions.
     *
     * Stores the descriptor locations, resource types, and non-owning GPU
     * resource handles associated with a material.
     *
     * The collection is independent of rendering backends and does not
     * own GPU resources or descriptor allocations.
     *
     * Binding compatibility and resource existence must be validated
     * separately by the consuming renderer.
     */
    struct MaterialResourceBindingSet {
        /** @brief Material resource binding descriptions owned by this
         * collection. */
        std::vector<MaterialResourceBinding> bindings;

        /**
         * @brief Compares two material resource binding collections.
         *
         * @param other Collection to compare.
         * @return True when both collections contain identical bindings
         * in the same order.
         */
        [[nodiscard]] constexpr auto
        operator==(const MaterialResourceBindingSet &other) const
            -> bool = default;
    };
} // namespace SNE::Engine::Renderer
