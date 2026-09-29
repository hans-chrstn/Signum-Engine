#pragma once

#include "engine/renderer/presentation_preference.hpp"
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
     * Owns the root of the Vulkan feature chain used when creating the logical
     * device. Additional Vulkan-version or extension feature structures may be
     * linked through the root as feature requirements are introduced.
     *
     * This configuration is distinct from PhysicalDeviceCapabilities, which
     * describes features supported by the physical device. A supported feature
     * is not necessarily enabled by this configuration.
     */
    struct LogicalDeviceFeatureConfiguration {
        /** Root of the Vulkan feature chain used for logical-device creation.
         */
        VkPhysicalDeviceFeatures2 feature_chain_root{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = nullptr,
            .features = {},
        };

        /** FIFO latest-ready feature configuration owned by this object. */
        VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR
            fifo_latest_ready_feature{
                .sType =
                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_MODE_FIFO_LATEST_READY_FEATURES_KHR,
                .pNext = nullptr,
                .presentModeFifoLatestReady = VK_FALSE,
            };
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
} // namespace SNE::Engine::Renderer::Vulkan
