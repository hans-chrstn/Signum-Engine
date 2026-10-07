#pragma once

#include "engine/renderer/vulkan/device/vulkan_device_capabilities.hpp"
#include "engine/renderer/vulkan/device/vulkan_queue_families.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_swapchain_support.hpp"
#include <optional>
#include <span>
#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {

    /**
     * @brief Represents a physical device being evaluated for renderer use.
     *
     * Groups the Vulkan physical-device handle with discovered queue-family
     * information, device-extension properties, capability information, and
     * swapchain support required by physical-device suitability and selection
     * policy.
     *
     * Queue-family indices represent discovered capabilities and remain
     * optional because this structure describes an evaluated device before
     * selection has completed.
     *
     * The structure does not own the Vulkan physical device. The handle remains
     * valid only while the Vulkan instance that provided it remains valid.
     *
     * @note Successful physical-device selection converts this discovered state
     * into SelectedPhysicalDevice, where required queue-family guarantees are
     * represented explicitly.
     */
    struct DiscoveredPhysicalDevice {
        /** Non-owning handle to the Vulkan physical device. */
        VkPhysicalDevice handle;
        /** Queue-family indices discovered for the physical device. */
        QueueFamilyIndices queue_family_indices;
        /**
         * Queue-family properties reported by Vulkan for the physical device.
         *
         * Contains hardware information such as queue capabilities and queue
         * counts used during physical-device selection.
         */
        std::vector<VkQueueFamilyProperties> queue_family_properties;
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
     * Stores the Vulkan physical-device handle, selected queue-family
     * information, and capability information required after physical-device
     * selection.
     *
     * The selected queue families are guaranteed to satisfy the renderer's
     * required graphics and presentation roles because this type is created
     * only after suitability validation succeeds.
     *
     * Each selected queue family also preserves the queue capacity reported by
     * Vulkan for later logical-device queue planning.
     *
     * The structure does not own the Vulkan physical device. The handle remains
     * valid only while the Vulkan instance that provided it remains valid.
     */
    struct SelectedPhysicalDevice {
        /** Non-owning handle to the selected Vulkan physical device. */
        VkPhysicalDevice handle;
        /**
         * Selected graphics and presentation queue families guaranteed by
         * physical-device selection.
         */
        SelectedQueueFamilies queue_families;
        /** Capabilities reported by the selected physical device. */
        PhysicalDeviceCapabilities capabilities;
    };

    /**
     * @brief Determines whether the available device extensions satisfy the
     * renderer's required device-extension policy.
     *
     * @param available_extensions Contiguous sequence of device-extension
     * properties reported by a physical device.
     *
     * @return true if all required device extensions are available; otherwise
     * false.
     */
    [[nodiscard]] auto supportsRequiredDeviceExtensions(
        std::span<const VkExtensionProperties> available_extensions) -> bool;

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
     * Vulkan API-version support, required renderer feature support, and
     * required swapchain support for a physical device.
     *
     * Required renderer features include Vulkan dynamic rendering and
     * synchronization2 functionality used by the current rendering path.
     *
     * @param queue_family_indices Queue-family indices discovered for the
     * device.
     * @param available_extensions Contiguous sequence of device-extension
     * properties reported by the device.
     * @param capabilities Capability information discovered for the device,
     * including mandatory renderer feature support.
     * @param swapchain_support Swapchain support discovered for the physical
     * device and surface.
     *
     * @return true if the device satisfies all mandatory renderer requirements;
     *         otherwise false.
     */
    [[nodiscard]] auto isPhysicalDeviceSuitable(
        const QueueFamilyIndices &queue_family_indices,
        std::span<const VkExtensionProperties> available_extensions,
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
     * @param discovered_devices Contiguous sequence of physical devices
     * previously inspected for renderer-relevant capabilities.
     *
     * @return Selected physical-device state when a suitable device is
     * available; otherwise std::nullopt.
     */
    [[nodiscard]] auto selectPhysicalDevice(
        std::span<const DiscoveredPhysicalDevice> discovered_devices)
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
     * @param devices Contiguous sequence of physical-device handles enumerated
     * from the Vulkan instance.
     * @param surface Vulkan surface used to evaluate presentation support and
     * discover swapchain capabilities.
     *
     * @return Inspected physical devices containing the information required
     * for selection.
     *
     * @throws Core::Error::EngineError if required Vulkan queries fail.
     */
    [[nodiscard]] auto
    inspectPhysicalDevices(std::span<const VkPhysicalDevice> devices,
                           VkSurfaceKHR surface)
        -> std::vector<DiscoveredPhysicalDevice>;
} // namespace SNE::Engine::Renderer::Vulkan
