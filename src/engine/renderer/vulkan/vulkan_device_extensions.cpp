#include "vulkan_device_extensions.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "vulkan_device_features.hpp"
#include "vulkan_result.hpp"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace {
    const std::vector<const char *> kRequiredDeviceExtensions{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};
}

namespace SNE::Engine::Renderer::Vulkan {
    auto queryDeviceExtensionProperties(VkPhysicalDevice device)
        -> std::vector<VkExtensionProperties> {
        std::vector<VkExtensionProperties> extension_properties{};
        while (true) {
            std::uint32_t count{};

            const VkResult count_result = vkEnumerateDeviceExtensionProperties(
                device, nullptr, &count, nullptr);

            if (count_result != VK_SUCCESS) {
                throw Core::Error::EngineError(
                    Core::Error::Code::VulkanExtensionEnumerationFailed,
                    "Failed to enumerate Vulkan device extensions",
                    Core::Error::NativeError(
                        static_cast<int>(count_result),
                        std::string(toString(count_result))),
                    "Enumerate Vulkan Device Extensions");
            }

            if (count == 0) {
                return extension_properties;
            }

            extension_properties.resize(count);

            const VkResult result = vkEnumerateDeviceExtensionProperties(
                device, nullptr, &count, extension_properties.data());

            if (result == VK_SUCCESS) {
                extension_properties.resize(count);
                break;
            }

            if (result == VK_INCOMPLETE) {
                continue;
            }

            throw Core::Error::EngineError(
                Core::Error::Code::VulkanExtensionEnumerationFailed,
                "Failed to enumerate Vulkan device extensions",
                Core::Error::NativeError(static_cast<int>(result),
                                         std::string(toString(result))),
                "Enumerate Vulkan Device Extensions");
        }

        return extension_properties;
    }

    auto requiredDeviceExtensions() -> const std::vector<const char *> & {
        return kRequiredDeviceExtensions;
    }

    auto deriveEnabledDeviceExtensions(
        const LogicalDeviceFeatureConfiguration &configuration)
        -> std::vector<const char *> {
        std::vector<const char *> extensions = requiredDeviceExtensions();

        extensions.reserve(extensions.size() + 1);

        if (configuration.fifo_latest_ready_feature
                .presentModeFifoLatestReady == VK_TRUE) {
            extensions.push_back(
                VK_KHR_PRESENT_MODE_FIFO_LATEST_READY_EXTENSION_NAME);
        }

        return extensions;
    }
} // namespace SNE::Engine::Renderer::Vulkan
