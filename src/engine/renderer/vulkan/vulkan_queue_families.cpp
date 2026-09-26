#include "vulkan_queue_families.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "vulkan_result.hpp"
#include <string>

namespace SNE::Engine::Renderer::Vulkan {
    auto queryQueueFamilyProperties(VkPhysicalDevice device)
        -> std::vector<VkQueueFamilyProperties> {
        std::vector<VkQueueFamilyProperties2> queried_properties{};
        std::uint32_t property_count{};
        vkGetPhysicalDeviceQueueFamilyProperties2(device, &property_count,
                                                  nullptr);

        if (property_count == 0) {
            return {};
        }

        queried_properties.resize(property_count);

        for (VkQueueFamilyProperties2 &property : queried_properties) {
            property.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
        }

        vkGetPhysicalDeviceQueueFamilyProperties2(device, &property_count,
                                                  queried_properties.data());

        queried_properties.resize(property_count);

        std::vector<VkQueueFamilyProperties> properties{};
        properties.reserve(queried_properties.size());

        for (const VkQueueFamilyProperties2 &property : queried_properties) {
            properties.push_back(property.queueFamilyProperties);
        }

        return properties;
    }

    auto findGraphicsQueueFamily(
        const std::vector<VkQueueFamilyProperties> &properties)
        -> std::optional<std::uint32_t> {
        for (std::size_t i{}; i < properties.size(); ++i) {
            // VK_QUEUE_GRAPHICS_BIT occupies the 0001 bit.
            // Bitwise AND checks whether that capability is present in
            // queueFlags.
            if ((properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
                return static_cast<std::uint32_t>(i);
            }
        }

        return std::nullopt;
    }

    auto supportsPresentation(VkPhysicalDevice device,
                              std::uint32_t queue_family_index,
                              VkSurfaceKHR surface) -> bool {
        VkBool32 supported{};
        const VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(
            device, queue_family_index, surface, &supported);

        if (result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanSurfaceSupportQueryFailed,
                "Failed to query Vulkan surface presentation support",
                Core::Error::NativeError(static_cast<int>(result),
                                         std::string(toString(result))),
                "Query Vulkan Surface Support");
        }

        return static_cast<bool>(supported);
    }

    auto findPresentationQueueFamily(
        VkPhysicalDevice device, VkSurfaceKHR surface,
        const std::vector<VkQueueFamilyProperties> &properties)
        -> std::optional<std::uint32_t> {

        for (std::size_t i{}; i < properties.size(); ++i) {
            const auto queue_family_index = static_cast<std::uint32_t>(i);
            if (supportsPresentation(device, queue_family_index, surface)) {
                return queue_family_index;
            }
        }

        return std::nullopt;
    }

    auto findQueueFamilyIndices(
        VkPhysicalDevice device, VkSurfaceKHR surface,
        const std::vector<VkQueueFamilyProperties> &properties)
        -> QueueFamilyIndices {
        const std::optional<std::uint32_t> graphics_family =
            findGraphicsQueueFamily(properties);
        const std::optional<std::uint32_t> presentation_family =
            findPresentationQueueFamily(device, surface, properties);
        return {.graphics_family = graphics_family,
                .presentation_family = presentation_family};
    }
} // namespace SNE::Engine::Renderer::Vulkan
