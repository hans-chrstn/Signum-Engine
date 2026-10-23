#pragma once

#include "engine/renderer/texture/texture_address_mode.hpp"
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Converts a backend-independent texture address mode to Vulkan.
     *
     * Maps a Signum texture addressing mode to its corresponding
     * VkSamplerAddressMode representation.
     *
     * The conversion does not require a Vulkan device or create
     * GPU resources.
     *
     * @param mode Backend-independent texture addressing mode.
     *
     * @return Corresponding Vulkan sampler address mode.
     *
     * @pre mode must identify a supported TextureAddressMode value.
     *
     * @note Unsupported address mode values trigger a fatal precondition
     * assertion in the Vulkan subsystem.
     */
    [[nodiscard]] auto toVkSamplerAddressMode(TextureAddressMode mode)
        -> VkSamplerAddressMode;
} // namespace SNE::Engine::Renderer::Vulkan
