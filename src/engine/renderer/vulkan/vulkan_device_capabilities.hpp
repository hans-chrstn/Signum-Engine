#pragma once
#include <vulkan/vulkan.h>
namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Stores capability information reported for a Vulkan physical
     * device.
     *
     * Groups the core properties and supported features queried from a physical
     * device so later systems can inspect device characteristics without
     * performing the Vulkan queries again.
     */
    struct PhysicalDeviceCapabilities {
        VkPhysicalDeviceProperties properties{};
        VkPhysicalDeviceFeatures features{};
    };

    /**
     * @brief Queries descriptive properties of a Vulkan physical device.
     *
     * Retrieves device identity, type, API version, driver information,
     * hardware limits, and other properties reported by Vulkan.
     *
     * @param device Physical device whose properties are queried.
     *
     * @return Properties reported for the physical device.
     */
    [[nodiscard]] auto queryPhysicalDeviceProperties(VkPhysicalDevice device)
        -> VkPhysicalDeviceProperties;

    /**
     * @brief Queries supported features of a Vulkan physical device.
     *
     * Retrieves the Vulkan features supported by the physical device for use
     * during capability discovery and later feature negotiation.
     *
     * @param device Physical device whose supported features are queried.
     *
     * @return Features supported by the physical device.
     */
    [[nodiscard]] auto queryPhysicalDeviceFeatures(VkPhysicalDevice device)
        -> VkPhysicalDeviceFeatures;

    /**
     * @brief Queries the core capabilities of a Vulkan physical device.
     *
     * Collects the physical device's descriptive properties and supported core
     * features into a single capability snapshot.
     *
     * @param device Physical device whose capabilities are queried.
     *
     * @return Capability information reported for the physical device.
     */
    [[nodiscard]] auto queryPhysicalDeviceCapabilities(VkPhysicalDevice device)
        -> PhysicalDeviceCapabilities;
} // namespace SNE::Engine::Renderer::Vulkan
