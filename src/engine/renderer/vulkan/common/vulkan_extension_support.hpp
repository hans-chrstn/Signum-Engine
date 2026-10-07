#pragma once

#include <span>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Reports whether all required Vulkan extensions are available.
     *
     * Compares the required extension names against the Vulkan extension
     * properties reported by the queried environment.
     *
     * @param required_extensions Contiguous sequence of extension names
     * required by Signum.
     * @param available_extensions Contiguous sequence of Vulkan extension
     * properties available from the queried environment.
     *
     * @return true if every required extension is available; otherwise false.
     */
    [[nodiscard]] auto hasRequiredExtensions(
        std::span<const char *const> required_extensions,
        std::span<const VkExtensionProperties> available_extensions) -> bool;
} // namespace SNE::Engine::Renderer::Vulkan
