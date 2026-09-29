#include "vulkan_device.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include "vulkan_device_extensions.hpp"
#include "vulkan_result.hpp"
#include <cstdint>
#include <string>

namespace SNE::Engine::Renderer::Vulkan {
    VulkanDevice::VulkanDevice(
        VkPhysicalDevice physical_device,
        const QueueFamilyIndices &queue_family_indices,
        const std::vector<QueueFamilyRequest> &queue_family_requests,
        const LogicalDeviceFeatureConfiguration &logical_device_configuration) {

        if (!queue_family_indices.graphics_family.has_value() ||
            !queue_family_indices.presentation_family.has_value()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanDevice requires graphics and presentation queue "
                "families");
        }

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos{};
        const std::uint32_t graphics_family =
            queue_family_indices.graphics_family.value();
        const std::uint32_t presentation_family =
            queue_family_indices.presentation_family.value();
        queue_create_infos.reserve(queue_family_requests.size());

        const float queue_priority = 1.0F;
        for (const QueueFamilyRequest &queue_family_request :
             queue_family_requests) {
            VkDeviceQueueCreateInfo queue_create_info{};
            queue_create_info.sType =
                VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueCount = std::uint32_t{1};
            queue_create_info.queueFamilyIndex =
                queue_family_request.family_index;
            queue_create_info.pQueuePriorities = &queue_priority;
            queue_create_infos.push_back(queue_create_info);
        }

        VkDeviceCreateInfo device_create_info{};
        const std::vector<const char *> &extensions =
            requiredDeviceExtensions();
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.queueCreateInfoCount =
            static_cast<std::uint32_t>(queue_create_infos.size());
        device_create_info.pQueueCreateInfos = queue_create_infos.data();
        device_create_info.enabledExtensionCount =
            static_cast<std::uint32_t>(extensions.size());
        device_create_info.ppEnabledExtensionNames = extensions.data();
        device_create_info.pEnabledFeatures =
            &logical_device_configuration.core_features;

        const VkResult result = vkCreateDevice(
            physical_device, &device_create_info, nullptr, &m_Device);

        if (result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanDeviceCreationFailed,
                "Failed to create a Vulkan device",
                Core::Error::NativeError(static_cast<int>(result),
                                         std::string(toString(result))),
                "Create Vulkan Device");
        }

        vkGetDeviceQueue(m_Device, graphics_family, std::uint32_t{0},
                         &m_GraphicsQueue);

        vkGetDeviceQueue(m_Device, presentation_family, std::uint32_t{0},
                         &m_PresentationQueue);
    }

    VulkanDevice::~VulkanDevice() noexcept {
        if (m_Device != VK_NULL_HANDLE) {
            vkDestroyDevice(m_Device, nullptr);
        }
    }

    auto VulkanDevice::nativeHandle() const noexcept -> VkDevice {
        return m_Device;
    }

    auto VulkanDevice::graphicsQueue() const noexcept -> VkQueue {
        return m_GraphicsQueue;
    }

    auto VulkanDevice::presentationQueue() const noexcept -> VkQueue {
        return m_PresentationQueue;
    }
} // namespace SNE::Engine::Renderer::Vulkan
