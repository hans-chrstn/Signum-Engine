#include "engine/renderer/vulkan/vulkan_device_capabilities.hpp"
#include "engine/renderer/vulkan/vulkan_device_discovery.hpp"
#include "engine/renderer/vulkan/vulkan_device_extensions.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <vulkan/vulkan.h>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

class VulkanDeviceCapabilitiesIntegrationTests : public ::testing::Test {
  protected:
    VkInstance m_Instance{VK_NULL_HANDLE};
    VkPhysicalDevice m_PhysicalDevice{VK_NULL_HANDLE};

    auto SetUp() -> void override;
    auto TearDown() -> void override;
};

auto VulkanDeviceCapabilitiesIntegrationTests::SetUp() -> void {
    VkApplicationInfo application_info{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &application_info;

    const VkResult result =
        vkCreateInstance(&create_info, nullptr, &m_Instance);

    ASSERT_EQ(result, VK_SUCCESS);

    const std::vector<VkPhysicalDevice> devices =
        Vulkan::enumeratePhysicalDevices(m_Instance);

    if (devices.empty()) {
        GTEST_SKIP() << "No Vulkan physical devices available";
    }

    m_PhysicalDevice = devices.front();
}

auto VulkanDeviceCapabilitiesIntegrationTests::TearDown() -> void {
    m_PhysicalDevice = VK_NULL_HANDLE;

    if (m_Instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_Instance, nullptr);
    }

    m_Instance = VK_NULL_HANDLE;
}

TEST_F(VulkanDeviceCapabilitiesIntegrationTests,
       QueriesPhysicalDeviceCapabilitiesFromPhysicalDevice) {
    ASSERT_NE(m_Instance, VK_NULL_HANDLE);
    ASSERT_NE(m_PhysicalDevice, VK_NULL_HANDLE);

    const std::vector<VkExtensionProperties> extensions =
        Vulkan::queryDeviceExtensionProperties(m_PhysicalDevice);

    const Vulkan::PhysicalDeviceCapabilities capabilities =
        Vulkan::queryPhysicalDeviceCapabilities(m_PhysicalDevice, extensions);

    EXPECT_NE(capabilities.properties.deviceName[0], '\0');
}
