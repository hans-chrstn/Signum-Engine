#pragma once

#include <span>
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
        /** Whether the FIFO latest-ready device extension is supported. */
        bool fifo_latest_ready_extension_supported = false;
        /** Whether FIFO latest-ready presentation is supported by the device.
         */
        bool fifo_latest_ready_feature_supported = false;
        /** Whether the Vulkan memory-budget extension is supported. */
        bool memory_budget_extension_supported = false;
    };

    /**
     * @brief Stores mandatory Vulkan capabilities required by the renderer.
     *
     * Represents Vulkan features that must be supported by a physical device
     * for the current renderer backend to operate correctly.
     */
    struct RequiredDeviceCapabilities {
        /** Whether Vulkan dynamic rendering is supported. */
        bool dynamic_rendering_supported = false;
        /** Whether Vulkan synchronization2 functionality is supported. */
        bool synchronization2_supported = false;
    };

    /**
     * @brief Stores capability information reported for a Vulkan physical
     * device.
     *
     * Groups the core properties, supported features, required capabilities,
     * and optional capabilities discovered for a physical device so later
     * systems can inspect device characteristics without repeating Vulkan
     * capability queries.
     */
    struct PhysicalDeviceCapabilities {
        /** Descriptive properties reported by the physical device. */
        VkPhysicalDeviceProperties properties{};
        /** Core Vulkan features supported by the physical device. */
        VkPhysicalDeviceFeatures features{};
        /** Mandatory capabilities required by the renderer. */
        RequiredDeviceCapabilities required_capabilities{};
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
     * @brief Queries optional Vulkan capabilities supported by a physical
     * device.
     *
     * Discovers optional Vulkan functionality that Signum may use when
     * available but does not require for physical-device suitability or
     * renderer startup.
     *
     * Combines Vulkan feature support with the supplied device-extension
     * information to determine whether optional capabilities such as FIFO
     * latest-ready presentation, memory-budget reporting, acceleration
     * structures, ray-tracing pipelines, and ray queries are reported as
     * supported.
     *
     * @param device Physical device whose optional capabilities are queried.
     * @param extension_properties Contiguous sequence of device-extension
     * properties previously discovered for the physical device.
     *
     * @return Optional capabilities supported by the physical device.
     */
    [[nodiscard]] auto queryOptionalDeviceCapabilities(
        VkPhysicalDevice device,
        std::span<const VkExtensionProperties> extension_properties)
        -> OptionalDeviceCapabilities;

    /**
     * @brief Queries mandatory Vulkan capabilities required by the renderer.
     *
     * Discovers Vulkan features that must be supported by a physical device for
     * the current renderer backend to operate correctly.
     *
     * Queries the Vulkan feature set required by the renderer, including
     * dynamic rendering and synchronization2 support.
     *
     * This function reports support only. It does not enable the discovered
     * features on a logical device.
     *
     * @param device Physical device whose required capabilities are queried.
     *
     * @return Required capabilities supported by the physical device.
     */
    [[nodiscard]] auto queryRequiredDeviceCapabilities(VkPhysicalDevice device)
        -> RequiredDeviceCapabilities;

    /**
     * @brief Queries the capabilities of a Vulkan physical device.
     *
     * Collects the physical device's descriptive properties, supported core
     * features, required renderer capabilities, and optional hardware
     * capabilities into a single capability snapshot.
     *
     * Required capabilities describe Vulkan functionality that must be
     * supported for the current renderer backend to operate correctly. Optional
     * capabilities describe functionality that may be used when available but
     * is not required for physical-device suitability.
     *
     * Reuses the supplied device-extension properties when determining optional
     * capability support.
     *
     * @param device Physical device whose capabilities are queried.
     * @param extension_properties Contiguous sequence of device-extension
     * properties previously discovered for the physical device.
     *
     * @return Capability information reported for the physical device.
     */
    [[nodiscard]] auto queryPhysicalDeviceCapabilities(
        VkPhysicalDevice device,
        std::span<const VkExtensionProperties> extension_properties)
        -> PhysicalDeviceCapabilities;
} // namespace SNE::Engine::Renderer::Vulkan
