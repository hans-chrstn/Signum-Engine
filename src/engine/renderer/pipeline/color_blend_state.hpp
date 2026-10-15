#pragma once

#include "engine/renderer/pipeline/blend_factor.hpp"
#include "engine/renderer/pipeline/blend_operation.hpp"
#include "engine/renderer/pipeline/color_write_mask.hpp"

namespace SNE::Engine::Renderer {
    /**
     * @brief Describes backend-independent color blending state.
     *
     * Defines how fragment output values are combined with existing
     * color attachment values and which components may be written.
     *
     * Blending is disabled by default. The configured factors and
     * operations are used when blending is enabled.
     */
    struct ColorBlendState {
        /**
         * @brief Enables blending between source and destination values.
         *
         * When disabled, fragment output replaces the destination value
         * for each enabled color component.
         */
        bool enabled{false};

        /** @brief Factor applied to the source color components. */
        BlendFactor source_color_factor{BlendFactor::SrcAlpha};

        /** @brief Factor applied to the destination color components. */
        BlendFactor destination_color_factor{BlendFactor::OneMinusSrcAlpha};

        /** @brief Operation used to combine the scaled color components. */
        BlendOperation color_operation{BlendOperation::Add};

        /** @brief Factor applied to the source alpha component. */
        BlendFactor source_alpha_factor{BlendFactor::One};

        /** @brief Factor applied to the destination alpha component. */
        BlendFactor destination_alpha_factor{BlendFactor::OneMinusSrcAlpha};

        /** @brief Operation used to combine the scaled alpha components. */
        BlendOperation alpha_operation{BlendOperation::Add};

        /**
         * @brief Selects which color attachment components may be written.
         *
         * All color components are writable by default.
         */
        ColorWriteMask write_mask{};

        /**
         * @brief Compares two color blending configurations.
         *
         * Compares blending enablement, source and destination factors,
         * blending operations, and color write masks.
         *
         * @param other Color blending state to compare.
         *
         * @return True if all blending configuration members are equal.
         */
        [[nodiscard]] constexpr auto
        operator==(const ColorBlendState &other) const noexcept
            -> bool = default;
    };
} // namespace SNE::Engine::Renderer
