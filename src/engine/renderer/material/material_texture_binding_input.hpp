#pragma once

#include "engine/renderer/texture/texture_upload_data.hpp"
#include <cstdint>
namespace SNE::Engine::Renderer {
    /**
     * @brief Describes a texture supplied to a material shader binding.
     *
     * Specifies the descriptor location and CPU-side texture data used
     * to initialize a sampled texture for a material.
     *
     * The texture upload description contains borrowed pixel storage.
     * The caller must keep that storage valid until the consuming upload
     * operation completes.
     *
     * This structure is independent of rendering backends and does not
     * own GPU resources or descriptor allocations.
     */
    struct MaterialTextureBindingInput {
        /** @brief Descriptor-set index containing the texture binding. */
        std::uint32_t set{};

        /** @brief Binding index within the descriptor set. */
        std::uint32_t binding{};

        /** @brief Element index within the descriptor binding array. */
        std::uint32_t array_element{};

        /**
         * @brief Texture data and sampling configuration.
         *
         * Pixel storage is borrowed and must remain valid until the
         * renderer completes the texture upload.
         */
        TextureUploadData texture{};
    };
} // namespace SNE::Engine::Renderer
