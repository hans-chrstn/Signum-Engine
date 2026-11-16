#pragma once

#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/renderer_capabilities.hpp"
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    struct PhysicalDeviceCapabilities;

    /**
     * @brief Describes optional Vulkan features requested by engine policy.
     *
     * Represents features Signum would like to enable before support on the
     * selected physical device is considered. Requested features are enabled
     * only when the required Vulkan capabilities are available.
     */
    struct LogicalDeviceFeatureRequest {
        /** Whether FIFO latest-ready presentation is requested. */
        bool fifo_latest_ready_requested = false;
    };

    /**
     * @brief Stores Vulkan features selected for logical-device creation.
     *
     * Owns the root of the Vulkan feature chain together with the
     * Vulkan-version and extension feature structures required during
     * logical-device creation.
     *
     * Required renderer features are enabled unconditionally after
     * physical-device selection has established that the selected device
     * supports them. Optional features are enabled only when requested by
     * engine policy and supported by the selected physical device.
     *
     * This configuration is distinct from PhysicalDeviceCapabilities, which
     * describes features supported by the physical device. Supported
     * optional features are not necessarily enabled by this configuration.
     */
    struct LogicalDeviceFeatureConfiguration {
        /** Root of the Vulkan feature chain used for logical-device creation.
         */
        VkPhysicalDeviceFeatures2 feature_chain_root{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = nullptr,
            .features = {},
        };

        /**
         * @brief Vulkan 1.3 features required by the renderer.
         *
         * Enables dynamic rendering and synchronization2 functionality used by
         * the renderer's command-recording and synchronization paths.
         */
        VkPhysicalDeviceVulkan13Features vulkan_13_features{};

        /** FIFO latest-ready feature configuration owned by this object. */
        VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR
            fifo_latest_ready_feature{
                .sType =
                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_MODE_FIFO_LATEST_READY_FEATURES_KHR,
                .pNext = nullptr,
                .presentModeFifoLatestReady = VK_FALSE,
        };

        /**
         * @brief Creates the logical-device feature configuration.
         *
         * Initializes the Vulkan feature structures owned by this configuration
         * and enables the mandatory Vulkan 1.3 features required by the
         * renderer.
         *
         * Optional features remain disabled until selected by
         * feature-negotiation policy.
         */
        LogicalDeviceFeatureConfiguration();
    };

    /**
     * @brief Derives Vulkan feature requests from renderer presentation policy.
     *
     * Translates the backend-independent presentation preference into optional
     * Vulkan feature requests needed to implement that behavior.
     *
     * This function expresses engine policy only. It does not query hardware
     * support or determine whether requested features can actually be enabled.
     *
     * @param presentation_preference Presentation behavior requested from the
     * renderer.
     *
     * @return Vulkan feature requests required by the selected presentation
     * policy.
     */
    [[nodiscard]] auto deriveLogicalDeviceFeatureRequest(
        PresentationPreference presentation_preference)
        -> LogicalDeviceFeatureRequest;

    /**
     * @brief Derives the Vulkan feature configuration for logical-device
     * creation.
     *
     * Determines which optional Vulkan features should be enabled by combining
     * engine feature requests with the capabilities reported by the selected
     * physical device.
     *
     * A requested feature is enabled only when all Vulkan support required by
     * that feature is available. The returned configuration contains feature
     * values only; its Vulkan pNext chain is not assembled by this function.
     *
     * @param request Optional Vulkan features requested by engine policy.
     * @param capabilities Capabilities reported by the selected physical
     * device.
     *
     * @return Logical-device feature configuration containing the features
     * selected for enablement.
     */
    [[nodiscard]] auto deriveLogicalDeviceFeatureConfiguration(
        const LogicalDeviceFeatureRequest &request,
        const PhysicalDeviceCapabilities &capabilities)
        -> LogicalDeviceFeatureConfiguration;

    /**
     * @brief Derives backend-independent renderer capabilities from Vulkan
     * state.
     *
     * Translates discovered physical-device support and negotiated
     * logical-device features into renderer capability descriptions.
     *
     * Optional Vulkan functionality is reported as enabled only when
     * the required device functionality is active.
     *
     * The current renderer enables memory-budget extension support when
     * available but does not enable ray-tracing functionality.
     *
     * @param physical_capabilities Capabilities discovered for the
     * selected Vulkan physical device.
     * @param enabled_features Features negotiated for logical-device creation.
     *
     * @return Backend-independent capability support and enablement.
     *
     * @pre The supplied states must describe the same selected device.
     *
     * @post Input configurations remain unchanged.
     *
     * @note This function does not query Vulkan, enable device features,
     * allocate GPU resources, or modify logical-device configuration.
     */
    [[nodiscard]] auto deriveRendererCapabilities(
        const PhysicalDeviceCapabilities &physical_capabilities,
        const LogicalDeviceFeatureConfiguration &enabled_features) noexcept
        -> RendererCapabilities;
} // namespace SNE::Engine::Renderer::Vulkan
