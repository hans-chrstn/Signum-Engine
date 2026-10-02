#include "vulkan_device.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include "vulkan_device_extensions.hpp"
#include "vulkan_device_features.hpp"
#include "vulkan_queue_families.hpp"
#include "vulkan_result.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace SNE::Engine::Renderer::Vulkan {
    VulkanDevice::VulkanDevice(
        VkPhysicalDevice physical_device,
        const SelectedQueueFamilies &queue_families,
        const std::vector<QueueFamilyRequest> &queue_family_requests,
        const LogicalDeviceFeatureConfiguration &logical_device_configuration) {

        const std::uint32_t graphics_family =
            queue_families.graphics_family.family_index;
        const std::uint32_t presentation_family =
            queue_families.presentation_family.family_index;

        const bool has_graphics_request = std::ranges::any_of(
            queue_family_requests,
            [graphics_family](const QueueFamilyRequest &request) -> bool {
                return request.family_index == graphics_family;
            });

        const bool has_presentation_request = std::ranges::any_of(
            queue_family_requests,
            [presentation_family](const QueueFamilyRequest &request) -> bool {
                return request.family_index == presentation_family;
            });

        if (!has_graphics_request || !has_presentation_request) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanDevice requires queue requests for graphics and "
                "presentation families");
        }

        for (std::size_t i{}; i < queue_family_requests.size(); ++i) {
            for (std::size_t j{i + 1}; j < queue_family_requests.size(); ++j) {
                if (queue_family_requests[i].family_index ==
                    queue_family_requests[j].family_index) {
                    Core::Assertion::failAssertion(
                        Core::Assertion::AssertionType::Precondition,
                        Core::Error::Subsystem::Vulkan,
                        "VulkanDevice requires each queue-family request to "
                        "use a unique queue-family index");
                }
            }
        }

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos{};
        queue_create_infos.reserve(queue_family_requests.size());

        for (const QueueFamilyRequest &queue_family_request :
             queue_family_requests) {
            VkDeviceQueueCreateInfo queue_create_info{};
            queue_create_info.sType =
                VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

            if (queue_family_request.priorities.empty()) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanDevice requires each queue-family request to "
                    "contain at least one queue priority");
            }

            if (queue_family_request.priorities.size() >
                std::numeric_limits<std::uint32_t>::max()) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanDevice queue-family request count exceeds the "
                    "Vulkan queue-count representation");
            }

            for (const float &priority : queue_family_request.priorities) {
                if (std::isnan(priority) || priority < 0.0F ||
                    priority > 1.0F) {
                    Core::Assertion::failAssertion(
                        Core::Assertion::AssertionType::Precondition,
                        Core::Error::Subsystem::Vulkan,
                        "VulkanDevice requires each queue priority to be "
                        "within the range [0.0, 1.0]");
                }
            }

            const auto queue_count = static_cast<std::uint32_t>(
                queue_family_request.priorities.size());

            queue_create_info.queueCount = queue_count;
            queue_create_info.queueFamilyIndex =
                queue_family_request.family_index;
            queue_create_info.pQueuePriorities =
                queue_family_request.priorities.data();
            queue_create_infos.push_back(queue_create_info);
        }

        LogicalDeviceFeatureConfiguration creation_features =
            logical_device_configuration;
        creation_features.feature_chain_root.pNext =
            &creation_features.vulkan_13_features;
        creation_features.fifo_latest_ready_feature.pNext = nullptr;
        creation_features.vulkan_13_features.pNext = nullptr;
        if (creation_features.fifo_latest_ready_feature
                .presentModeFifoLatestReady == VK_TRUE) {
            creation_features.vulkan_13_features.pNext =
                &creation_features.fifo_latest_ready_feature;
        }

        VkDeviceCreateInfo device_create_info{};
        const std::vector<const char *> extensions =
            deriveEnabledDeviceExtensions(logical_device_configuration);
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.queueCreateInfoCount =
            static_cast<std::uint32_t>(queue_create_infos.size());
        device_create_info.pQueueCreateInfos = queue_create_infos.data();
        device_create_info.enabledExtensionCount =
            static_cast<std::uint32_t>(extensions.size());
        device_create_info.ppEnabledExtensionNames = extensions.data();
        device_create_info.pNext = &creation_features.feature_chain_root;
        device_create_info.pEnabledFeatures = nullptr;

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
