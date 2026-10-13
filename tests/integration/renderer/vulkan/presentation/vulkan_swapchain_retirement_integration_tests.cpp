#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/vulkan/device/vulkan_device.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_discovery.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_features.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_selection.hpp"
#include "engine/renderer/vulkan/device/vulkan_queue_requests.hpp"
#include "engine/renderer/vulkan/presentation/fail_closed_replacement.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_surface.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_swapchain.hpp"
#include "engine/renderer/vulkan/vulkan_instance.hpp"

#include <cstdint>
#include <cstdlib>
#include <gtest/gtest.h>
#include <optional>
#include <stdexcept>
#include <vulkan/vulkan.h>

namespace Platform = SNE::Engine::Platform;
namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

TEST(VulkanSwapchainRetirementIntegrationTests,
     ClearsRetiredSwapchainAfterReplacementConstructionFails) {
    if (std::getenv("DISPLAY") == nullptr &&
        std::getenv("WAYLAND_DISPLAY") == nullptr) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw{};

    const std::uint32_t width = 640;
    const std::uint32_t height = 480;

    const Platform::Window window{
        {.width = width, .height = height},
        "Signum Swapchain Retirement Test",
    };

    Vulkan::VulkanInstance instance{"Signum Swapchain Test", false};

    Vulkan::VulkanSurface surface{
        instance.nativeHandle(),
        window.nativeHandle(),
    };

    const auto available_devices =
        Vulkan::enumeratePhysicalDevices(instance.nativeHandle());

    const auto discovered_devices = Vulkan::inspectPhysicalDevices(
        available_devices, surface.nativeHandle());

    const auto selected_device =
        Vulkan::selectPhysicalDevice(discovered_devices);

    if (!selected_device.has_value()) {
        GTEST_SKIP() << "No suitable presentation-capable Vulkan device";
    }

    const auto feature_request = Vulkan::deriveLogicalDeviceFeatureRequest(
        Renderer::PresentationPreference::VSync);

    const auto feature_configuration =
        Vulkan::deriveLogicalDeviceFeatureConfiguration(
            feature_request, selected_device->capabilities);

    const auto queue_requests = Vulkan::deriveUniqueQueueFamilyRequests(
        selected_device->queue_families);

    Vulkan::VulkanDevice device{
        selected_device->handle,
        selected_device->queue_families,
        queue_requests,
        feature_configuration,
        selected_device->capabilities.optional_capabilities,
    };

    const auto make_swapchain =
        [&](VkSwapchainKHR old_swapchain) -> Vulkan::VulkanSwapchain {
        return Vulkan::VulkanSwapchain{
            selected_device->handle,
            device.nativeHandle(),
            surface.nativeHandle(),
            selected_device->queue_families,
            window.framebufferSize(),
            Renderer::PresentationPreference::VSync,
            false,
            old_swapchain,
        };
    };

    std::optional<Vulkan::VulkanSwapchain> current;

    current.emplace(make_swapchain(VK_NULL_HANDLE));

    ASSERT_TRUE(current.has_value());
    ASSERT_NE(current->nativeHandle(), VK_NULL_HANDLE);

    const VkSwapchainKHR old_handle = current->nativeHandle();

    bool replacement_created = false;

    EXPECT_THROW(Vulkan::replaceRetiringResource(
                     current,
                     [&]() -> Vulkan::VulkanSwapchain {
                         Vulkan::VulkanSwapchain replacement =
                             make_swapchain(old_handle);

                         replacement_created =
                             replacement.nativeHandle() != VK_NULL_HANDLE;

                         throw std::runtime_error{
                             "Deliberate failure after replacement creation"};
                     }),
                 std::runtime_error);

    EXPECT_TRUE(replacement_created);
    EXPECT_FALSE(current.has_value());

    device.waitIdle();
}
