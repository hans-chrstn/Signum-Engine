#include "vulkan_device_selection.hpp"
#include "engine/renderer/vulkan/vulkan_api_version.hpp"
#include "engine/renderer/vulkan/vulkan_device_capabilities.hpp"
#include "engine/renderer/vulkan/vulkan_device_extensions.hpp"
#include "engine/renderer/vulkan/vulkan_extension_support.hpp"
#include "engine/renderer/vulkan/vulkan_queue_families.hpp"
#include <optional>
#include <utility>
#include <vector>

namespace {
    const std::vector<const char *> kRequiredDeviceExtensions{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};
}

namespace SNE::Engine::Renderer::Vulkan {
    auto supportsRequiredDeviceExtensions(
        const std::vector<VkExtensionProperties> &available_extensions)
        -> bool {
        return hasRequiredExtensions(kRequiredDeviceExtensions,
                                     available_extensions);
    }

    auto isPhysicalDeviceSuitable(
        const QueueFamilyIndices &queue_family_indices,
        const std::vector<VkExtensionProperties> &available_extensions,
        const PhysicalDeviceCapabilities &capabilities) -> bool {

        return queue_family_indices.graphics_family.has_value() &&
               queue_family_indices.presentation_family.has_value() &&
               supportsRequiredDeviceExtensions(available_extensions) &&
               supportsRequiredApiVersion(capabilities.properties.apiVersion);
    }

    auto selectPhysicalDevice(
        const std::vector<PhysicalDeviceCandidate> &device_candidates)
        -> std::optional<PhysicalDeviceCandidate> {
        for (const PhysicalDeviceCandidate &device_candidate :
             device_candidates) {
            if (isPhysicalDeviceSuitable(device_candidate.queue_family_indices,
                                         device_candidate.available_extensions,
                                         device_candidate.capabilities)) {
                return device_candidate;
            }
        }
        return std::nullopt;
    }

    auto
    createPhysicalDeviceCandidates(const std::vector<VkPhysicalDevice> &devices,
                                   VkSurfaceKHR surface)
        -> std::vector<PhysicalDeviceCandidate> {

        std::vector<PhysicalDeviceCandidate> candidates{};

        candidates.reserve(devices.size());

        for (const VkPhysicalDevice &device : devices) {
            std::vector<VkExtensionProperties> extension_properties =
                queryDeviceExtensionProperties(device);

            const std::vector<VkQueueFamilyProperties> queue_family_properties =
                queryQueueFamilyProperties(device);

            const PhysicalDeviceCapabilities capabilities =
                queryPhysicalDeviceCapabilities(device, extension_properties);

            const QueueFamilyIndices queue_family_indices =
                findQueueFamilyIndices(device, surface,
                                       queue_family_properties);

            PhysicalDeviceCandidate candidate{
                .handle = device,
                .queue_family_indices = queue_family_indices,
                .available_extensions = std::move(extension_properties),
                .capabilities = capabilities};

            candidates.push_back(std::move(candidate));
        }

        return candidates;
    }
} // namespace SNE::Engine::Renderer::Vulkan
