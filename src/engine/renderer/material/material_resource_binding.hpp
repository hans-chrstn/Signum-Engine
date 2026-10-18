#pragma once

#include "engine/renderer/resource/gpu_resource_handle.hpp"
#include "engine/renderer/shader/shader_resource_type.hpp"
#include <cstdint>

namespace SNE::Engine::Renderer {
    /**
     * @brief Identifies a GPU resource supplied to a material shader binding.
     *
     * Associates a shader descriptor location and resource type with a
     * non-owning GPU resource handle.
     *
     * The binding is independent of rendering backends and does not own
     * GPU resources or descriptor allocations.
     *
     * Resource existence and compatibility with shader declarations must
     * be validated by the consuming renderer.
     */
    struct MaterialResourceBinding {
        /** @brief Descriptor-set index containing the binding. */
        std::uint32_t set{};

        /** @brief Binding index within the descriptor set. */
        std::uint32_t binding{};

        /** @brief Element index within the descriptor binding's array. */
        std::uint32_t array_element{};

        /** @brief Shader resource type expected at this binding. */
        Shader::ShaderResourceType type{
            Shader::ShaderResourceType::CombinedImageSampler,
        };

        /**
         * @brief Non-owning handle identifying the supplied GPU resource.
         *
         * A default-initialized handle does not identify a registered resource.
         */
        GpuResourceHandle resource_handle{};

        /**
         * @brief Compares material resource bindings.
         *
         * @param other Binding to compare.
         * @return True when all binding fields and resource identities match.
         */
        [[nodiscard]] constexpr auto
        operator==(const MaterialResourceBinding &other) const noexcept
            -> bool = default;
    };
} // namespace SNE::Engine::Renderer
