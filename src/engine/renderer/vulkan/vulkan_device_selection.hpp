#pragma once

#include "engine/renderer/vulkan/vulkan_device_capabilities.hpp"
#include "engine/renderer/vulkan/vulkan_queue_families.hpp"
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Represents a physical device being evaluated for renderer use.
     *
     * Groups the Vulkan physical-device handle with the discovered queue-family
     * indices, device-extension properties, and capability information required
     * by physical-device suitability and selection policy.
     *
     * The structure does not own the Vulkan physical device. The handle remains
     * valid only while the Vulkan instance that provided it remains valid.
     */
    struct PhysicalDeviceCandidate {
        /** Non-owning handle to the Vulkan physical device. */
        VkPhysicalDevice handle;
        /** Queue-family indices discovered for the physical device. */
        QueueFamilyIndices queue_family_indices;
        /** Device extensions reported as available by the physical device. */
        std::vector<VkExtensionProperties> available_extensions;
        /** Capabilities reported by the physical device. */
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
     * @brief Determines whether discovered physical-device capabilities satisfy
     *        the renderer's mandatory requirements.
     *
     * Evaluates required queue-family availability, device-extension support,
     * and Vulkan API-version support for a physical device.
     *
     * @param queue_family_indices Queue-family indices discovered for the
     * device.
     * @param available_extensions Device extensions reported by the device.
     * @param capabilities Capability information discovered for the device.
     *
     * @return true if the device satisfies all mandatory renderer requirements;
     *         otherwise false.
     */
    [[nodiscard]] auto isPhysicalDeviceSuitable(
        const QueueFamilyIndices &queue_family_indices,
        const std::vector<VkExtensionProperties> &available_extensions,
        const PhysicalDeviceCapabilities &capabilities) -> bool;

    /**
     * @brief Selects a suitable physical device candidate for renderer use.
     *
     * Evaluates the supplied physical-device candidates using the renderer's
     * physical-device suitability policy and returns the first candidate that
     * satisfies the required queue-family, device-extension, and Vulkan
     * API-version requirements.
     *
     * This function does not enumerate physical devices or query Vulkan device
     * capabilities. Candidate creation and capability discovery are expected
     * to occur before selection.
     *
     * @param device_candidates Physical-device candidates previously created
     *                           from Vulkan physical-device discovery.
     *
     * @return The selected physical-device candidate when a suitable device is
     *         available; otherwise std::nullopt.
     */
    [[nodiscard]] auto selectPhysicalDevice(
        const std::vector<PhysicalDeviceCandidate> &device_candidates)
        -> std::optional<PhysicalDeviceCandidate>;

    /**
     * @brief Creates physical-device candidates from discovered Vulkan devices.
     *
     * Queries queue-family information and available device-extension
     * properties for each discovered physical device and constructs
     * PhysicalDeviceCandidate structures used by physical-device selection.
     *
     * This function does not determine whether a device is suitable.
     * Suitability evaluation is performed separately by the physical-device
     * selection policy.
     *
     * @param devices Physical devices discovered from the Vulkan instance.
     * @param surface Vulkan surface used to evaluate presentation queue
     * support.
     *
     * @return Physical-device candidates containing the discovered information
     *         required for selection.
     *
     * @throws Core::Error::EngineError if required Vulkan queries fail.
     */
    [[nodiscard]] auto
    createPhysicalDeviceCandidates(const std::vector<VkPhysicalDevice> &devices,
                                   VkSurfaceKHR surface)
        -> std::vector<PhysicalDeviceCandidate>;
} // namespace SNE::Engine::Renderer::Vulkan
