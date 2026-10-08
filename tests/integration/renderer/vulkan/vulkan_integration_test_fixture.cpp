#include "vulkan_integration_test_fixture.hpp"
#include "engine/renderer/vulkan/common/vulkan_api_version.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_discovery.hpp"
#include "engine/renderer/vulkan/device/vulkan_queue_families.hpp"
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

auto VulkanIntegrationTest::SetUp() -> void {
    std::uint32_t supported_api_version{};
    const VkResult api_result =
        vkEnumerateInstanceVersion(&supported_api_version);

    if (api_result != VK_SUCCESS) {
        FAIL() << "Failed to query Vulkan loader API version";
    }

    if (!Vulkan::supportsRequiredApiVersion(supported_api_version)) {
        GTEST_SKIP() << "Vulkan loader does not support Signum's required API "
                        "version";
    }

    VkApplicationInfo application_info{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.apiVersion = Vulkan::kRequiredApiVersion;

    VkInstanceCreateInfo instance_create_info{};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &application_info;

    const VkResult instance_result =
        vkCreateInstance(&instance_create_info, nullptr, &m_Instance);

    ASSERT_EQ(instance_result, VK_SUCCESS);

    const std::vector<VkPhysicalDevice> devices =
        Vulkan::enumeratePhysicalDevices(m_Instance);

    if (devices.empty()) {
        GTEST_SKIP() << "No Vulkan physical devices available";
    }

    m_PhysicalDevice = devices.front();

    const std::vector<VkQueueFamilyProperties> queue_family_properties =
        Vulkan::queryQueueFamilyProperties(m_PhysicalDevice);

    const std::optional<std::uint32_t> graphics_family =
        Vulkan::findGraphicsQueueFamily(queue_family_properties);

    if (!graphics_family.has_value()) {
        FAIL() << "Failed to find a graphics queue family";
    }

    const std::uint32_t graphics_family_index = graphics_family.value();

    m_SelectedQueueFamily.family_index = graphics_family_index;
    m_SelectedQueueFamily.available_queue_count =
        queue_family_properties[graphics_family_index].queueCount;

    const float priority = 1.0F;

    VkDeviceQueueCreateInfo device_queue_info{};
    device_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    device_queue_info.queueFamilyIndex = graphics_family_index;
    device_queue_info.queueCount = 1U;
    device_queue_info.pQueuePriorities = &priority;

    VkPhysicalDeviceVulkan13Features vulkan13_features{};
    vulkan13_features.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    vulkan13_features.synchronization2 = VK_TRUE;

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount = 1U;
    device_create_info.pQueueCreateInfos = &device_queue_info;
    device_create_info.pNext = &vulkan13_features;

    const VkResult device_result = vkCreateDevice(
        m_PhysicalDevice, &device_create_info, nullptr, &m_Device);

    ASSERT_EQ(device_result, VK_SUCCESS);

    vkGetDeviceQueue(m_Device, m_SelectedQueueFamily.family_index, 0U,
                     &m_GraphicsQueue);

    ASSERT_NE(m_GraphicsQueue, VK_NULL_HANDLE);

    m_MemoryAllocator.emplace(m_Instance, m_PhysicalDevice, m_Device,
                              Vulkan::kRequiredApiVersion, false);

    m_ImmediateSubmission.emplace(m_Device, m_GraphicsQueue,
                                  m_SelectedQueueFamily.family_index);
}

auto VulkanIntegrationTest::TearDown() -> void {
    m_ImmediateSubmission.reset();

    m_MemoryAllocator.reset();

    if (m_Device != VK_NULL_HANDLE) {
        vkDestroyDevice(m_Device, nullptr);
    }

    if (m_Instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_Instance, nullptr);
    }

    m_GraphicsQueue = VK_NULL_HANDLE;
    m_PhysicalDevice = VK_NULL_HANDLE;
    m_Device = VK_NULL_HANDLE;
    m_Instance = VK_NULL_HANDLE;
}

auto VulkanIntegrationTest::memoryAllocator()
    -> Vulkan::VulkanMemoryAllocator & {
    if (!m_MemoryAllocator.has_value()) {
        throw std::logic_error(
            "VulkanIntegrationTest requires an initialized memory allocator");
    }

    return m_MemoryAllocator.value();
}

auto VulkanIntegrationTest::immediateSubmission()
    -> Vulkan::VulkanImmediateSubmission & {
    if (!m_ImmediateSubmission.has_value()) {
        throw std::logic_error("VulkanIntegrationTest requires an initialized "
                               "immediate submission");
    }

    return m_ImmediateSubmission.value();
}
