#pragma once

#include <span>
#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Reports whether all required Vulkan extensions are available.
     *
     * Compares the required extension names against a collection of available
     * Vulkan extension properties.
     *
     * @param required_extensions Extension names required by Signum.
     * @param available_extensions Vulkan extensions available from the queried
     * environment.
     *
     * @return true if every required extension is available; otherwise false.
     */
    [[nodiscard]] auto hasRequiredExtensions(
        std::span<const char *const> required_extensions,
        const std::vector<VkExtensionProperties> &available_extensions) -> bool;
} // namespace SNE::Engine::Renderer::Vulkan
