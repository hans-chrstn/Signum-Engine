#include "vulkan_swapchain_support.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/renderer/vulkan/common/vulkan_result.hpp"
#include <cstdint>
#include <string>

namespace SNE::Engine::Renderer::Vulkan {
    auto querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
        -> SwapchainSupportDetails {
        SwapchainSupportDetails details{};

        const VkResult capabilities_result =
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                device, surface, &details.surface_capabilities);
        if (capabilities_result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanSwapchainSupportQueryFailed,
                "Failed to query Vulkan swapchain surface capabilities",
                Core::Error::NativeError(
                    static_cast<int>(capabilities_result),
                    std::string(toString(capabilities_result))),
                "Query Vulkan Swapchain Surface Capabilities");
        }

        while (true) {
            std::uint32_t format_count{};
            const VkResult count_result = vkGetPhysicalDeviceSurfaceFormatsKHR(
                device, surface, &format_count, nullptr);

            if (count_result != VK_SUCCESS) {
                throw Core::Error::EngineError(
                    Core::Error::Code::VulkanSwapchainSupportQueryFailed,
                    "Failed to obtain Vulkan swapchain surface formats",
                    Core::Error::NativeError(
                        static_cast<int>(count_result),
                        std::string(toString(count_result))),
                    "Query Vulkan Swapchain Surface Formats");
            }

            if (format_count == 0) {
                break;
            }

            details.available_surface_formats.resize(format_count);

            const VkResult formats_result =
                vkGetPhysicalDeviceSurfaceFormatsKHR(
                    device, surface, &format_count,
                    details.available_surface_formats.data());

            if (formats_result == VK_SUCCESS) {
                details.available_surface_formats.resize(format_count);
                break;
            }

            if (formats_result == VK_INCOMPLETE) {
                continue;
            }

            throw Core::Error::EngineError(
                Core::Error::Code::VulkanSwapchainSupportQueryFailed,
                "Failed to obtain Vulkan swapchain surface formats",
                Core::Error::NativeError(static_cast<int>(formats_result),
                                         std::string(toString(formats_result))),
                "Query Vulkan Swapchain Surface Formats");
        }

        while (true) {
            std::uint32_t presentation_mode_count{};
            const VkResult count_result =
                vkGetPhysicalDeviceSurfacePresentModesKHR(
                    device, surface, &presentation_mode_count, nullptr);

            if (count_result != VK_SUCCESS) {
                throw Core::Error::EngineError(
                    Core::Error::Code::VulkanSwapchainSupportQueryFailed,
                    "Failed to obtain Vulkan swapchain surface present modes",
                    Core::Error::NativeError(
                        static_cast<int>(count_result),
                        std::string(toString(count_result))),
                    "Query Vulkan Swapchain Surface Present Modes");
            }

            if (presentation_mode_count == 0) {
                break;
            }

            details.available_presentation_modes.resize(
                presentation_mode_count);

            const VkResult presentation_modes_result =
                vkGetPhysicalDeviceSurfacePresentModesKHR(
                    device, surface, &presentation_mode_count,
                    details.available_presentation_modes.data());

            if (presentation_modes_result == VK_SUCCESS) {
                details.available_presentation_modes.resize(
                    presentation_mode_count);
                break;
            }

            if (presentation_modes_result == VK_INCOMPLETE) {
                continue;
            }

            throw Core::Error::EngineError(
                Core::Error::Code::VulkanSwapchainSupportQueryFailed,
                "Failed to obtain Vulkan swapchain surface present modes",
                Core::Error::NativeError(
                    static_cast<int>(presentation_modes_result),
                    std::string(toString(presentation_modes_result))),
                "Query Vulkan Swapchain Surface Present Modes");
        }

        return details;
    }
} // namespace SNE::Engine::Renderer::Vulkan
