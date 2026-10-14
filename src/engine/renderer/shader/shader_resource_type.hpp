#pragma once

#include <cstdint>

namespace SNE::Engine::Renderer::Shader {
    /**
     * @brief Identifies the resource type required by a shader binding.
     *
     * Describes shader resource requirements independently of graphics APIs.
     * Rendering backends translate these values into their native
     * descriptor types.
     */
    enum class ShaderResourceType : std::uint8_t {
        /** @brief Texture and sampler accessed through a combined binding. */
        CombinedImageSampler,

        /** @brief Read-only uniform buffer containing shader parameters. */
        UniformBuffer,

        /** @brief Buffer supporting shader storage reads and writes. */
        StorageBuffer,
    };
} // namespace SNE::Engine::Renderer::Shader
