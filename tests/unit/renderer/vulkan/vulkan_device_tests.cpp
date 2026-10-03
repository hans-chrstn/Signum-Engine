#include "engine/renderer/vulkan/vulkan_device.hpp"
#include "engine/renderer/vulkan/vulkan_device_features.hpp"
#include "engine/renderer/vulkan/vulkan_queue_families.hpp"
#include "engine/renderer/vulkan/vulkan_queue_requests.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <vulkan/vulkan.h>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

TEST(VulkanDeviceTests, RequiresGraphicsQueueRequest) {
    std::byte dummy{};
    auto *physical_device = reinterpret_cast<VkPhysicalDevice>(&dummy);
    std::vector<float> priorities{};
    priorities.push_back(1.0F);

    const Vulkan::SelectedQueueFamilies queue_families{
        .graphics_family =
            {
                .family_index = 0U,
                .available_queue_count = 1U,
            },
        .presentation_family =
            {
                .family_index = 1U,
                .available_queue_count = 1U,
            },
    };

    const std::vector<Vulkan::QueueFamilyRequest> queue_family_requests{
        Vulkan::QueueFamilyRequest{
            .family_index = 1U,
            .priorities = priorities,
        },
    };

    const Vulkan::LogicalDeviceFeatureConfiguration
        logical_device_configuration{};

    ASSERT_DEATH(
        Vulkan::VulkanDevice(physical_device, queue_families,
                             queue_family_requests,
                             logical_device_configuration),
        "VulkanDevice requires queue requests for graphics and presentation "
        "families");
}

TEST(VulkanDeviceTests, RequiresPresentationQueueRequest) {
    std::byte dummy{};
    auto *physical_device = reinterpret_cast<VkPhysicalDevice>(&dummy);
    std::vector<float> priorities{};
    priorities.push_back(1.0F);

    const Vulkan::SelectedQueueFamilies queue_families{
        .graphics_family =
            {
                .family_index = 0U,
                .available_queue_count = 1U,
            },
        .presentation_family =
            {
                .family_index = 1U,
                .available_queue_count = 1U,
            },
    };

    const std::vector<Vulkan::QueueFamilyRequest> queue_family_requests{
        Vulkan::QueueFamilyRequest{
            .family_index = 0U,
            .priorities = priorities,
        },
    };

    const Vulkan::LogicalDeviceFeatureConfiguration
        logical_device_configuration{};

    ASSERT_DEATH(
        Vulkan::VulkanDevice(physical_device, queue_families,
                             queue_family_requests,
                             logical_device_configuration),
        "VulkanDevice requires queue requests for graphics and presentation "
        "families");
}
