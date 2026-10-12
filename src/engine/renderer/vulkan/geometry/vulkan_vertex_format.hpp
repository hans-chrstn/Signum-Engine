#pragma once

#include "engine/renderer/geometry/vertex_format.hpp"
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Converts a backend-independent vertex format to a Vulkan format.
     *
     * Maps a Signum vertex attribute format to its corresponding
     * Vulkan VkFormat representation.
     *
     * The conversion is independent of Vulkan devices and does not
     * create or access GPU resources.
     *
     * @param format Backend-independent vertex attribute format.
     *
     * @return Corresponding Vulkan vertex attribute format.
     *
     * @pre format must identify a supported VertexFormat value.
     *
     * @note Unsupported format values violate the function's
     * precondition and trigger a fatal assertion.
     */
    [[nodiscard]] auto toVkFormat(VertexFormat format) -> VkFormat;
} // namespace SNE::Engine::Renderer::Vulkan
