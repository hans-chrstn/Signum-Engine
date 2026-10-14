#pragma once

#include "engine/renderer/shader/shader_resource_type.hpp"
#include <cstdint>

namespace SNE::Engine::Renderer::Shader {
    /**
     * @brief Describes a resource binding required by a shader.
     *
     * Identifies the descriptor set, binding index, resource type,
     * and number of descriptors required by a shader entry point.
     *
     * The description is independent of graphics APIs and does
     * not own GPU resources.
     */
    struct ShaderDescriptorBinding {
        /**
         * @brief Descriptor set index containing the resource binding.
         */
        std::uint32_t set{};

        /**
         * @brief Binding index within the descriptor set.
         */
        std::uint32_t binding{};

        /**
         * @brief Type of shader resource required by this binding.
         */
        ShaderResourceType type{};

        /**
         * @brief Number of descriptors required by this binding.
         *
         * Must be greater than zero for a valid descriptor binding.
         */
        std::uint32_t count{};
    };
} // namespace SNE::Engine::Renderer::Shader
