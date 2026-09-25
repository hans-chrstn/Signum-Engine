#include "vulkan_device_discovery.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/native_error.hpp"
#include "vulkan_result.hpp"
#include <cstdint>
#include <string>

namespace SNE::Engine::Renderer::Vulkan {
    auto enumeratePhysicalDevices(VkInstance instance)
        -> std::vector<VkPhysicalDevice> {
        std::vector<VkPhysicalDevice> available_devices{};
        while (true) {
            std::uint32_t device_count{};
            const VkResult count_result =
                vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
            if (count_result != VK_SUCCESS) {
                throw Core::Error::EngineError(
                    Core::Error::Code::VulkanPhysicalDeviceEnumerationFailed,
                    "Failed to enumerate Vulkan physical devices",
                    Core::Error::NativeError(
                        static_cast<int>(count_result),
                        std::string(toString(count_result))),
                    "Enumerate Vulkan Physical Devices");
            }

            if (device_count == 0) {
                return available_devices;
            }

            available_devices.resize(device_count);

            const VkResult device_result = vkEnumeratePhysicalDevices(
                instance, &device_count, available_devices.data());
            if (device_result == VK_SUCCESS) {
                available_devices.resize(device_count);
                break;
            }
            if (device_result == VK_INCOMPLETE) {
                continue;
            }

            throw Core::Error::EngineError(
                Core::Error::Code::VulkanPhysicalDeviceEnumerationFailed,
                "Failed to enumerate Vulkan physical devices",
                Core::Error::NativeError(static_cast<int>(device_result),
                                         std::string(toString(device_result))),
                "Enumerate Vulkan Physical Devices");
        }

        return available_devices;
    }
} // namespace SNE::Engine::Renderer::Vulkan
