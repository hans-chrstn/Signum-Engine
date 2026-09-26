#include "vulkan_device_selection.hpp"
#include "engine/renderer/vulkan/vulkan_extension_support.hpp"
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
        const std::vector<VkExtensionProperties> &available_extensions)
        -> bool {

        return queue_family_indices.graphics_family.has_value() &&
               queue_family_indices.presentation_family.has_value() &&
               supportsRequiredDeviceExtensions(available_extensions);
    }

    auto selectPhysicalDevice(
        const std::vector<PhysicalDeviceCandidate> &device_candidates)
        -> std::optional<PhysicalDeviceCandidate> {
        for (const PhysicalDeviceCandidate &device_candidate :
             device_candidates) {
            if (isPhysicalDeviceSuitable(
                    device_candidate.queue_family_indices,
                    device_candidate.available_extensions)) {
                return device_candidate;
            }
        }
        return std::nullopt;
    }
} // namespace SNE::Engine::Renderer::Vulkan
