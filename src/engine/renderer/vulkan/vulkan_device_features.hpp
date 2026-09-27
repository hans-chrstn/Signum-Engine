#pragma once

#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Stores Vulkan features selected for logical-device creation.
     *
     * Represents the core Vulkan features that Signum has explicitly chosen to
     * enable for a logical device.
     *
     * This configuration is distinct from PhysicalDeviceCapabilities, which
     * describes features supported by the physical device. A supported feature
     * is not necessarily enabled by this configuration.
     */
    struct LogicalDeviceFeatureConfiguration {
        /** Core Vulkan features selected for logical-device creation. */
        VkPhysicalDeviceFeatures core_features{};
    };
} // namespace SNE::Engine::Renderer::Vulkan
