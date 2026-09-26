#pragma once
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
