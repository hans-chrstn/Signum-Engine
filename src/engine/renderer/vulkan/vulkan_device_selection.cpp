#include "vulkan_device_selection.hpp"
#include "vulkan_api_version.hpp"
#include "vulkan_device_capabilities.hpp"
#include "vulkan_device_extensions.hpp"
#include "vulkan_extension_support.hpp"
#include "vulkan_queue_families.hpp"
#include "vulkan_swapchain_support.hpp"
#include <optional>
#include <utility>
#include <vector>

namespace SNE::Engine::Renderer::Vulkan {
    auto hasAdequateSwapchainSupport(
        const SwapchainSupportDetails &swapchain_support) -> bool {
        return !swapchain_support.available_surface_formats.empty() &&
               !swapchain_support.available_presentation_modes.empty();
    }

    auto supportsRequiredDeviceExtensions(
        const std::vector<VkExtensionProperties> &available_extensions)
        -> bool {
        return hasRequiredExtensions(requiredDeviceExtensions(),
                                     available_extensions);
    }

    auto isPhysicalDeviceSuitable(
        const QueueFamilyIndices &queue_family_indices,
        const std::vector<VkExtensionProperties> &available_extensions,
        const PhysicalDeviceCapabilities &capabilities,
        const SwapchainSupportDetails &swapchain_support) -> bool {

        return queue_family_indices.graphics_family.has_value() &&
               queue_family_indices.presentation_family.has_value() &&
               supportsRequiredDeviceExtensions(available_extensions) &&
               supportsRequiredApiVersion(capabilities.properties.apiVersion) &&
               hasAdequateSwapchainSupport(swapchain_support);
    }

    auto selectPhysicalDevice(
        const std::vector<PhysicalDeviceCandidate> &device_candidates)
        -> std::optional<PhysicalDeviceCandidate> {
        for (const PhysicalDeviceCandidate &device_candidate :
             device_candidates) {
            if (isPhysicalDeviceSuitable(device_candidate.queue_family_indices,
                                         device_candidate.available_extensions,
                                         device_candidate.capabilities,
                                         device_candidate.swapchain_support)) {
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

            SwapchainSupportDetails swapchain_support =
                querySwapchainSupport(device, surface);

            PhysicalDeviceCandidate candidate{
                .handle = device,
                .queue_family_indices = queue_family_indices,
                .available_extensions = std::move(extension_properties),
                .capabilities = capabilities,
                .swapchain_support = std::move(swapchain_support),
            };

            candidates.push_back(std::move(candidate));
        }

        return candidates;
    }
} // namespace SNE::Engine::Renderer::Vulkan
