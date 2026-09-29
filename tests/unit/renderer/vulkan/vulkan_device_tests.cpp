#include "engine/renderer/vulkan/vulkan_device.hpp"
#include "engine/renderer/vulkan/vulkan_device_features.hpp"
#include "engine/renderer/vulkan/vulkan_queue_families.hpp"
#include "engine/renderer/vulkan/vulkan_queue_requests.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <vulkan/vulkan.h>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

TEST(VulkanDeviceTests, RequiresGraphicsQueueFamily) {
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    Vulkan::QueueFamilyIndices queue_family_indices{};
    queue_family_indices.presentation_family = 0U;
    const std::vector<Vulkan::QueueFamilyRequest> queue_family_requests{};
    const Vulkan::LogicalDeviceFeatureConfiguration
        logical_device_configuration{};

    ASSERT_DEATH(
        Vulkan::VulkanDevice(physical_device, queue_family_indices,
                             queue_family_requests,
                             logical_device_configuration),
        "VulkanDevice requires graphics and presentation queue families");
}

TEST(VulkanDeviceTests, RequiresPresentationQueueFamily) {
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    Vulkan::QueueFamilyIndices queue_family_indices{};
    queue_family_indices.graphics_family = 0U;
    const std::vector<Vulkan::QueueFamilyRequest> queue_family_requests{};
    const Vulkan::LogicalDeviceFeatureConfiguration
        logical_device_configuration{};

    ASSERT_DEATH(
        Vulkan::VulkanDevice(physical_device, queue_family_indices,
                             queue_family_requests,
                             logical_device_configuration),
        "VulkanDevice requires graphics and presentation queue families");
}
