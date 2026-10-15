#pragma once

#include "engine/renderer/pipeline/color_blend_state.hpp"
#include "engine/renderer/pipeline/compare_op.hpp"
#include "engine/renderer/pipeline/cull_mode.hpp"
#include "engine/renderer/pipeline/front_face.hpp"
#include "engine/renderer/pipeline/primitive_topology.hpp"

namespace SNE::Engine::Renderer {
    /**
     * @brief Describes backend-independent graphics pipeline state.
     *
     * Defines configurable rendering behavior without depending
     * on a specific graphics API.
     */
    struct GraphicsPipelineState {
        /** @brief Enables depth comparison during fragment processing. */
        bool depth_test_enabled{true};

        /** @brief Enables writing passing fragment depths to the depth buffer.
         */
        bool depth_write_enabled{true};

        /**
         * @brief Selects which triangle faces are culled.
         *
         * Defaults to no culling to preserve existing rendering behavior.
         */
        CullMode cull_mode{CullMode::None};

        /**
         * @brief Selects the triangle winding order considered front-facing.
         *
         * Defaults to counterclockwise to preserve existing pipeline behavior.
         */
        FrontFace front_face{FrontFace::CounterClockwise};

        /**
         * @brief Selects how vertices are assembled into primitives.
         *
         * Defaults to independent triangles to preserve existing rendering
         * behavior.
         */
        PrimitiveTopology topology{PrimitiveTopology::TriangleList};

        /**
         * @brief Selects the comparison operation used by depth testing.
         *
         * Defaults to less-than comparison to preserve existing rendering
         * behavior.
         */
        CompareOp depth_compare{CompareOp::Less};

        /**
         * @brief Configures color blending and color component writes.
         *
         * Blending is disabled by default to preserve existing rendering
         * behavior.
         */
        ColorBlendState color_blend{};

        /**
         * @brief Compares two graphics pipeline configurations.
         *
         * Compares all pipeline state members, including depth, rasterization,
         * primitive topology, and color blending settings.
         *
         * @param other Graphics pipeline state to compare.
         *
         * @return True if all configuration members are equal.
         */
        [[nodiscard]] constexpr auto
        operator==(const GraphicsPipelineState &other) const noexcept
            -> bool = default;
    };
} // namespace SNE::Engine::Renderer
