#pragma once

#include "engine/renderer/material/material_resource_binding_set.hpp"

namespace SNE::Engine::Renderer {
    /**
     * @brief Describes the resources used by a material instance.
     *
     * Stores backend-independent resource bindings for textures and buffers.
     *
     * Resource handles are non-owning. The renderer is responsible for
     * validating referenced resources and creating descriptor resources.
     */
    struct MaterialInstanceDescription {
        /**
         * @brief Resource bindings associated with the material instance.
         *
         * References registered GPU resources without owning them.
         */
        MaterialResourceBindingSet resource_bindings{};

        /**
         * @brief Compares two material instance descriptions.
         *
         * @param other Material instance description to compare.
         * @return True when both descriptions have identical resource bindings.
         */
        [[nodiscard]] constexpr auto
        operator==(const MaterialInstanceDescription &other) const
            -> bool = default;
    };
} // namespace SNE::Engine::Renderer
