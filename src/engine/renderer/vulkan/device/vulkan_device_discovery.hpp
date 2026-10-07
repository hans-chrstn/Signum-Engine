#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Enumerates the Vulkan physical devices available to an instance.
     *
     * Queries the Vulkan implementation for all physical devices accessible
     * through the provided instance.
     *
     * An empty result indicates that enumeration succeeded but no physical
     * devices were available.
     *
     * @param instance Vulkan instance used to discover physical devices.
     *
     * @return Physical-device handles reported by Vulkan.
     *
     * @throws Core::Error::EngineError if physical-device enumeration fails.
     */
    [[nodiscard]] auto enumeratePhysicalDevices(VkInstance instance)
        -> std::vector<VkPhysicalDevice>;
} // namespace SNE::Engine::Renderer::Vulkan
