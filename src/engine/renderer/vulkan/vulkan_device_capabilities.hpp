#pragma once

#include <vector>
#include <vulkan/vulkan.h>
namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Stores optional capabilities reported for a Vulkan physical
     * device.
     *
     * Represents hardware features that Signum may use when available but does
     * not require for renderer startup or physical-device suitability.
     *
     * Unsupported optional capabilities must not cause an otherwise suitable
     * physical device to be rejected.
     */
    struct OptionalDeviceCapabilities {
        /** Whether acceleration-structure functionality is supported. */
        bool acceleration_structures_supported = false;
        /** Whether Vulkan ray-tracing pipeline functionality is supported. */
        bool ray_tracing_pipeline_supported = false;
        /** Whether Vulkan ray-query functionality is supported. */
        bool ray_query_supported = false;
    };

    /**
     * @brief Stores capability information reported for a Vulkan physical
     * device.
     *
     * Groups the core properties, supported features, and optional capabilities
     * discovered for a physical device so later systems can inspect device
     * characteristics without repeating Vulkan capability queries.
     */
    struct PhysicalDeviceCapabilities {
        /** Descriptive properties reported by the physical device. */
        VkPhysicalDeviceProperties properties{};
        /** Core Vulkan features supported by the physical device. */
        VkPhysicalDeviceFeatures features{};
        /** Optional hardware capabilities discovered for the physical device.
         */
        OptionalDeviceCapabilities optional_capabilities;
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
     * @brief Queries optional hardware capabilities supported by a Vulkan
     * physical device.
     *
     * Discovers optional Vulkan features that Signum may use when available but
     * does not require for physical-device suitability or renderer startup.
     *
     * Combines Vulkan feature support with the supplied device-extension
     * information to determine whether optional capabilities such as
     * acceleration structures, ray-tracing pipelines, and ray queries are
     * reported as supported.
     *
     * @param device Physical device whose optional capabilities are queried.
     * @param extension_properties Device extensions previously discovered for
     * the physical device.
     *
     * @return Optional capabilities supported by the physical device.
     */
    [[nodiscard]] auto queryOptionalDeviceCapabilities(
        VkPhysicalDevice device,
        const std::vector<VkExtensionProperties> &extension_properties)
        -> OptionalDeviceCapabilities;

    /**
     * @brief Queries the capabilities of a Vulkan physical device.
     *
     * Collects the physical device's descriptive properties, supported core
     * features, and optional hardware capabilities into a single capability
     * snapshot.
     *
     * Reuses the supplied device-extension properties when determining optional
     * capability support.
     *
     * @param device Physical device whose capabilities are queried.
     * @param extension_properties Device extensions previously discovered for
     * the physical device.
     *
     * @return Capability information reported for the physical device.
     */
    [[nodiscard]] auto queryPhysicalDeviceCapabilities(
        VkPhysicalDevice device,
        const std::vector<VkExtensionProperties> &extension_properties)
        -> PhysicalDeviceCapabilities;
} // namespace SNE::Engine::Renderer::Vulkan
