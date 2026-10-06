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

namespace {

    [[nodiscard]] auto
    findQueueFamilyProperty(std::span<const VkQueueFamilyProperties> properties,
                            std::uint32_t family_index)
        -> const VkQueueFamilyProperties * {

        if (family_index >= properties.size()) {
            return nullptr;
        }

        return &properties[family_index];
    }

} // namespace

namespace SNE::Engine::Renderer::Vulkan {
    auto hasRequiredSwapchainSupport(
        const SwapchainSupportDetails &swapchain_support) -> bool {
        return !swapchain_support.available_surface_formats.empty() &&
               !swapchain_support.available_presentation_modes.empty();
    }

    auto supportsRequiredDeviceExtensions(
        std::span<const VkExtensionProperties> available_extensions) -> bool {
        return hasRequiredExtensions(requiredDeviceExtensions(),
                                     available_extensions);
    }

    auto isPhysicalDeviceSuitable(
        const QueueFamilyIndices &queue_family_indices,
        std::span<const VkExtensionProperties> available_extensions,
        const PhysicalDeviceCapabilities &capabilities,
        const SwapchainSupportDetails &swapchain_support) -> bool {
        return queue_family_indices.graphics_family.has_value() &&
               queue_family_indices.presentation_family.has_value() &&
               supportsRequiredDeviceExtensions(available_extensions) &&
               supportsRequiredApiVersion(capabilities.properties.apiVersion) &&
               capabilities.required_capabilities.dynamic_rendering_supported &&
               capabilities.required_capabilities.synchronization2_supported &&
               hasRequiredSwapchainSupport(swapchain_support);
    }

    auto selectPhysicalDevice(
        std::span<const DiscoveredPhysicalDevice> discovered_devices)
        -> std::optional<SelectedPhysicalDevice> {
        for (const DiscoveredPhysicalDevice &device : discovered_devices) {
            const std::optional<std::uint32_t> &graphics_family =
                device.queue_family_indices.graphics_family;

            const std::optional<std::uint32_t> &presentation_family =
                device.queue_family_indices.presentation_family;

            if (!graphics_family.has_value() ||
                !presentation_family.has_value()) {
                continue;
            }

            if (!isPhysicalDeviceSuitable(
                    device.queue_family_indices, device.available_extensions,
                    device.capabilities, device.swapchain_support)) {
                continue;
            }

            const VkQueueFamilyProperties *graphics_properties =
                findQueueFamilyProperty(device.queue_family_properties,
                                        graphics_family.value());

            const VkQueueFamilyProperties *presentation_properties =
                findQueueFamilyProperty(device.queue_family_properties,
                                        presentation_family.value());

            if (graphics_properties == nullptr ||
                presentation_properties == nullptr) {
                continue;
            }

            return SelectedPhysicalDevice{
                .handle = device.handle,
                .queue_families =
                    {
                        .graphics_family =
                            {
                                .family_index = graphics_family.value(),
                                .available_queue_count =
                                    graphics_properties->queueCount,
                            },

                        .presentation_family =
                            {
                                .family_index = presentation_family.value(),
                                .available_queue_count =
                                    presentation_properties->queueCount,
                            },
                    },
                .capabilities = device.capabilities,
            };
        }

        return std::nullopt;
    }

    auto inspectPhysicalDevices(std::span<const VkPhysicalDevice> devices,
                                VkSurfaceKHR surface)
        -> std::vector<DiscoveredPhysicalDevice> {
        std::vector<DiscoveredPhysicalDevice> discovered_devices{};

        discovered_devices.reserve(devices.size());

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

            DiscoveredPhysicalDevice discovered_device{
                .handle = device,
                .queue_family_indices = queue_family_indices,
                .queue_family_properties = queue_family_properties,
                .available_extensions = std::move(extension_properties),
                .capabilities = capabilities,
                .swapchain_support = std::move(swapchain_support),
            };

            discovered_devices.push_back(std::move(discovered_device));
        }

        return discovered_devices;
    }
} // namespace SNE::Engine::Renderer::Vulkan
