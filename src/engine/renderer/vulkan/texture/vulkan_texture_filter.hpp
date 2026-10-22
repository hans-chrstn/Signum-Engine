#pragma once

#include "engine/renderer/texture/texture_filter.hpp"
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Converts a backend-independent texture filter to a Vulkan filter.
     *
     * Maps a Signum texture filtering mode to its corresponding
     * VkFilter representation.
     *
     * The conversion does not require a Vulkan device or create
     * GPU resources.
     *
     * @param filter Backend-independent texture filtering mode.
     *
     * @return Corresponding Vulkan texture filter.
     *
     * @pre filter must identify a supported TextureFilter value.
     *
     * @note Unsupported filter values trigger a fatal precondition
     * assertion in the Vulkan subsystem.
     */
    [[nodiscard]] auto toVkFilter(TextureFilter filter) -> VkFilter;
} // namespace SNE::Engine::Renderer::Vulkan
