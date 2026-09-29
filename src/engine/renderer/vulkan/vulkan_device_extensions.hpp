#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    struct LogicalDeviceFeatureConfiguration;

    /**
     * @brief Queries the device extensions supported by a physical device.
     *
     * Retrieves the Vulkan device-extension properties reported for the
     * specified physical device.
     *
     * An empty result indicates that no device extensions were reported.
     *
     * @param device Physical device whose supported device extensions are
     * queried.
     *
     * @return Device-extension properties reported for the physical device.
     *
     * @throws Core::Error::EngineError if Vulkan fails to enumerate device
     *         extension properties.
     */
    [[nodiscard]] auto queryDeviceExtensionProperties(VkPhysicalDevice device)
        -> std::vector<VkExtensionProperties>;

    /**
     * @brief Returns the Vulkan device extensions required by Signum.
     *
     * Provides the shared device-extension policy used when validating physical
     * devices and when enabling extensions during logical-device creation.
     *
     * @return Required Vulkan device-extension names.
     */
    [[nodiscard]] auto requiredDeviceExtensions()
        -> const std::vector<const char *> &;

    /**
     * @brief Derives the Vulkan device extensions enabled for logical-device
     *        creation.
     *
     * Begins with the device extensions required by Signum and adds optional
     * extensions needed by features selected in the supplied logical-device
     * feature configuration.
     *
     * Optional extensions are enabled only when their corresponding feature has
     * already been selected for enablement. Capability discovery and feature
     * negotiation are expected to occur before this function is called.
     *
     * @param configuration Vulkan features selected for logical-device
     * creation.
     *
     * @return Vulkan device-extension names that should be enabled when
     * creating the logical device.
     */
    [[nodiscard]] auto deriveEnabledDeviceExtensions(
        const LogicalDeviceFeatureConfiguration &configuration)
        -> std::vector<const char *>;
} // namespace SNE::Engine::Renderer::Vulkan
