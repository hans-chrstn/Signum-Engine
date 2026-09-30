#pragma once

#include "engine/renderer/vulkan/vulkan_device_capabilities.hpp"
#include "engine/renderer/vulkan/vulkan_queue_families.hpp"
#include "vulkan_swapchain_support.hpp"
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Represents a physical device being evaluated for renderer use.
     *
     * Groups the Vulkan physical-device handle with the discovered queue-family
     * indices, device-extension properties, capability information, and
     * swapchain support required by physical-device suitability and selection
     * policy.
     *
     * The structure does not own the Vulkan physical device. The handle remains
     * valid only while the Vulkan instance that provided it remains valid.
     *
     * @note Queue-family indices remain optional because this type represents
     * discovered physical-device state. Successful physical-device selection
     * converts a suitable discovered device into SelectedPhysicalDevice, where
     * required queue-family guarantees are represented explicitly.
     */
    struct DiscoveredPhysicalDevice {
        /** Non-owning handle to the Vulkan physical device. */
        VkPhysicalDevice handle;
        /** Queue-family indices discovered for the physical device. */
        QueueFamilyIndices queue_family_indices;
        /** Device extensions reported as available by the physical device. */
        std::vector<VkExtensionProperties> available_extensions;
        /** Capabilities reported by the physical device. */
        PhysicalDeviceCapabilities capabilities;
        /** Swapchain support reported for the physical device and surface. */
        SwapchainSupportDetails swapchain_support;
    };

    /**
     * @brief Represents a physical device validated and selected for renderer
     * use.
     *
     * Stores the Vulkan physical-device handle, required queue-family indices,
     * and capability information needed after physical-device selection.
     *
     * The queue-family indices are guaranteed to contain the graphics and
     * presentation families required by the renderer because this type is
     * created only after the corresponding discovered physical device satisfies
     * the renderer's suitability policy.
     *
     * The structure does not own the Vulkan physical device. The handle remains
     * valid only while the Vulkan instance that provided it remains valid.
     */
    struct SelectedPhysicalDevice {
        /** Non-owning handle to the selected Vulkan physical device. */
        VkPhysicalDevice handle;
        /** Required queue-family indices guaranteed by device selection. */
        SelectedQueueFamilyIndices queue_family_indices;
        /** Capabilities reported by the selected physical device. */
        PhysicalDeviceCapabilities capabilities;
    };

    /**
     * @brief Determines whether the available device extensions satisfy the
     *        renderer's required device-extension policy.
     *
     * @param available_extensions Device extensions reported by a physical
     * device.
     *
     * @return true if all required device extensions are available; otherwise
     *         false.
     */
    [[nodiscard]] auto supportsRequiredDeviceExtensions(
        const std::vector<VkExtensionProperties> &available_extensions) -> bool;

    /**
     * @brief Determines whether swapchain support satisfies the renderer's
     *        presentation requirements.
     *
     * A physical device has required swapchain support when at least one
     * surface format and at least one presentation mode are available for the
     * associated Vulkan surface.
     *
     * @param swapchain_support Swapchain support discovered for the physical
     * device and surface.
     *
     * @return true if at least one surface format and presentation mode are
     *         available; otherwise false.
     */
    [[nodiscard]] auto hasRequiredSwapchainSupport(
        const SwapchainSupportDetails &swapchain_support) -> bool;

    /**
     * @brief Determines whether discovered physical-device capabilities satisfy
     *        the renderer's mandatory requirements.
     *
     * Evaluates required queue-family availability, device-extension support,
     * Vulkan API-version support, and required swapchain support for a physical
     * device.
     *
     * @param queue_family_indices Queue-family indices discovered for the
     * device.
     * @param available_extensions Device extensions reported by the device.
     * @param capabilities Capability information discovered for the device.
     * @param swapchain_support Swapchain support discovered for the physical
     * device and surface.
     *
     * @return true if the device satisfies all mandatory renderer requirements;
     *         otherwise false.
     */
    [[nodiscard]] auto isPhysicalDeviceSuitable(
        const QueueFamilyIndices &queue_family_indices,
        const std::vector<VkExtensionProperties> &available_extensions,
        const PhysicalDeviceCapabilities &capabilities,
        const SwapchainSupportDetails &swapchain_support) -> bool;

    /**
     * @brief Selects a suitable discovered physical device for renderer use.
     *
     * Evaluates the supplied discovered physical devices using the renderer's
     * suitability policy and returns selected physical-device state whose
     * required runtime invariants have been established.
     *
     * This function does not enumerate physical devices or query Vulkan device
     * capabilities. Physical-device inspection is expected to occur before
     * selection.
     *
     * @param discovered_devices Physical devices previously inspected for
     * renderer-relevant capabilities.
     *
     * @return Selected physical-device state when a suitable device is
     * available; otherwise std::nullopt.
     */
    [[nodiscard]] auto selectPhysicalDevice(
        const std::vector<DiscoveredPhysicalDevice> &discovered_devices)
        -> std::optional<SelectedPhysicalDevice>;

    /**
     * @brief Inspects discovered Vulkan physical devices for renderer use.
     *
     * Queries queue-family information, available device extensions,
     * physical-device capabilities, and swapchain support for each Vulkan
     * physical device and constructs DiscoveredPhysicalDevice structures used
     * by physical-device selection.
     *
     * This function does not determine whether a device is suitable.
     * Suitability evaluation is performed separately by the physical-device
     * selection policy.
     *
     * @param devices Physical-device handles enumerated from the Vulkan
     * instance.
     * @param surface Vulkan surface used to evaluate presentation support and
     * discover swapchain capabilities.
     *
     * @return Inspected physical devices containing the information required
     * for selection.
     *
     * @throws Core::Error::EngineError if required Vulkan queries fail.
     */
    [[nodiscard]] auto
    inspectPhysicalDevices(const std::vector<VkPhysicalDevice> &devices,
                           VkSurfaceKHR surface)
        -> std::vector<DiscoveredPhysicalDevice>;
} // namespace SNE::Engine::Renderer::Vulkan
