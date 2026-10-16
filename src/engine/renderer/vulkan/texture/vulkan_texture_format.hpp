#pragma once

#include "engine/renderer/texture/texture_format.hpp"
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Converts a backend-independent texture format to a Vulkan format.
     *
     * Maps a Signum texture format to its corresponding VkFormat
     * representation.
     *
     * The conversion does not require a Vulkan device or create
     * GPU resources.
     *
     * @param format Backend-independent texture format.
     *
     * @return Corresponding Vulkan texture format.
     *
     * @pre format must identify a supported TextureFormat value.
     *
     * @note Unsupported format values trigger a fatal precondition
     * assertion in the Vulkan subsystem.
     */
    [[nodiscard]] auto toVkFormat(TextureFormat format) -> VkFormat;
} // namespace SNE::Engine::Renderer::Vulkan
