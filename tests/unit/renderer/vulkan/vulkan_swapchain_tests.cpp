#include "engine/platform/window.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/vulkan/vulkan_queue_families.hpp"
#include "engine/renderer/vulkan/vulkan_swapchain.hpp"
#include <gtest/gtest.h>
#include <vulkan/vulkan.h>

namespace Platform = SNE::Engine::Platform;
namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = Renderer::Vulkan;

TEST(VulkanSwapchainTests, RequiresGraphicsQueueFamily) {
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice logical_device = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    Vulkan::QueueFamilyIndices queue_family_indices{};
    queue_family_indices.presentation_family = 0U;
    const Platform::FramebufferSize framebuffer_size{
        .width = 1280,
        .height = 720,
    };

    ASSERT_DEATH(
        Vulkan::VulkanSwapchain(physical_device, logical_device, surface,
                                queue_family_indices, framebuffer_size,
                                Renderer::PresentationPreference::VSync, false),
        "VulkanSwapchain requires graphics and presentation queue "
        "families");
}

TEST(VulkanSwapchainTests, RequiresPresentationQueueFamily) {
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice logical_device = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    Vulkan::QueueFamilyIndices queue_family_indices{};
    queue_family_indices.graphics_family = 0U;
    const Platform::FramebufferSize framebuffer_size{
        .width = 1280,
        .height = 720,
    };

    ASSERT_DEATH(
        Vulkan::VulkanSwapchain(physical_device, logical_device, surface,
                                queue_family_indices, framebuffer_size,
                                Renderer::PresentationPreference::VSync, false),
        "VulkanSwapchain requires graphics and presentation queue "
        "families");
}
