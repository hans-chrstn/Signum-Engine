#pragma once

#include "engine/renderer/material/material_buffer_update_frequency.hpp"
#include "engine/renderer/shader/shader_resource_type.hpp"
#include <cstddef>
#include <cstdint>
#include <span>

namespace SNE::Engine::Renderer {
    /**
     * @brief Describes initial data and shader binding information
     * for a material buffer.
     *
     * Specifies the shader descriptor location, buffer resource type,
     * and CPU data used to initialize a uniform or storage buffer.
     *
     * CPU data is borrowed and must remain valid until the consuming
     * upload operation completes.
     *
     * This structure does not own GPU resources or depend on Vulkan.
     */
    struct MaterialBufferBindingInput {
        /** @brief Descriptor-set index containing the buffer binding. */
        std::uint32_t set{};

        /** @brief Binding index within the descriptor set. */
        std::uint32_t binding{};

        /** @brief Element index within the descriptor binding's array. */
        std::uint32_t array_element{};

        /**
         * @brief Shader resource type of the buffer.
         *
         * Must identify either a uniform buffer or a storage buffer.
         */
        Shader::ShaderResourceType type{
            Shader::ShaderResourceType::UniformBuffer,
        };

        /** @brief Intended update frequency of the material buffer. */
        MaterialBufferUpdateFrequency update_frequency{
            MaterialBufferUpdateFrequency::Occasional,
        };

        /**
         * @brief Borrowed CPU data used to initialize the buffer.
         *
         * The caller must keep this storage valid until the consuming
         * operation completes.
         */
        std::span<const std::byte> data;
    };
} // namespace SNE::Engine::Renderer
