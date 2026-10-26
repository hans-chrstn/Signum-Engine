#include "engine/core/application.hpp"
#include "engine/core/application_configuration.hpp"
#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/vulkan/common/vulkan_api_version.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_discovery.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_selection.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_surface.hpp"
#include "engine/renderer/vulkan/vulkan_external_rendering_info.hpp"
#include "engine/renderer/vulkan/vulkan_instance.hpp"
#include "engine/renderer/vulkan/vulkan_renderer.hpp"
#include <cstdint>
#include <cstdlib>
#include <gtest/gtest.h>
#include <utility>
#include <vulkan/vulkan.h>

namespace Core = SNE::Engine::Core;
namespace Platform = SNE::Engine::Platform;
namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = Renderer::Vulkan;

namespace {
    constexpr Platform::WindowSize kWindowSize{
        .width = 640,
        .height = 480,
    };

    [[nodiscard]] auto hasDisplay() -> bool {
        return std::getenv("DISPLAY") != nullptr ||
               std::getenv("WAYLAND_DISPLAY") != nullptr;
    }

    [[nodiscard]] auto
    hasSuitablePresentationDevice(const Platform::Window &window) -> bool {
        Vulkan::VulkanInstance instance{
            "Signum External Rendering Integration Test",
            false,
        };

        Vulkan::VulkanSurface surface{
            instance.nativeHandle(),
            window.nativeHandle(),
        };

        const auto devices =
            Vulkan::enumeratePhysicalDevices(instance.nativeHandle());

        const auto discovered =
            Vulkan::inspectPhysicalDevices(devices, surface.nativeHandle());

        return Vulkan::selectPhysicalDevice(discovered).has_value();
    }
} // namespace

TEST(VulkanExternalRenderingInfoIntegrationTests,
     ReturnsValidRendererHandlesAndConfiguration) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw_context{};

    const Platform::Window window{
        kWindowSize,
        "Signum External Rendering Integration Test",
    };

    if (!hasSuitablePresentationDevice(window)) {
        GTEST_SKIP() << "No suitable presentation-capable Vulkan device";
    }

    Vulkan::VulkanRenderer renderer{
        "Signum External Rendering Integration Test",
        window,
        Renderer::PresentationPreference::VSync,
        false,
    };

    const Vulkan::VulkanExternalRenderingInfo info =
        renderer.externalRenderingInfo();

    EXPECT_NE(info.instance, VK_NULL_HANDLE);
    EXPECT_NE(info.physical_device, VK_NULL_HANDLE);
    EXPECT_NE(info.device, VK_NULL_HANDLE);
    EXPECT_NE(info.graphics_queue, VK_NULL_HANDLE);

    EXPECT_EQ(info.api_version, Vulkan::kRequiredApiVersion);

    EXPECT_NE(info.color_attachment_format, VK_FORMAT_UNDEFINED);
    EXPECT_GT(info.swapchain_image_count, 0U);

    std::uint32_t queue_family_count{};

    vkGetPhysicalDeviceQueueFamilyProperties(info.physical_device,
                                             &queue_family_count, nullptr);

    ASSERT_LT(info.graphics_queue_family_index, queue_family_count);

    VkQueue graphics_queue = VK_NULL_HANDLE;

    vkGetDeviceQueue(info.device, info.graphics_queue_family_index, 0U,
                     &graphics_queue);

    EXPECT_EQ(graphics_queue, info.graphics_queue);

    ASSERT_NO_THROW(renderer.renderFrame());

    const Vulkan::VulkanExternalRenderingInfo after_frame =
        renderer.externalRenderingInfo();

    EXPECT_EQ(after_frame.instance, info.instance);
    EXPECT_EQ(after_frame.physical_device, info.physical_device);
    EXPECT_EQ(after_frame.device, info.device);
    EXPECT_EQ(after_frame.graphics_queue, info.graphics_queue);
    EXPECT_EQ(after_frame.graphics_queue_family_index,
              info.graphics_queue_family_index);
    EXPECT_EQ(after_frame.api_version, info.api_version);
    EXPECT_EQ(after_frame.color_attachment_format,
              info.color_attachment_format);
    EXPECT_EQ(after_frame.swapchain_image_count, info.swapchain_image_count);
}

TEST(VulkanExternalRenderingInfoIntegrationTests,
     ApplicationForwardsRendererInformation) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    {
        Platform::GlfwContext glfw_context{};

        const Platform::Window window{
            kWindowSize,
            "Signum Application Capability Test",
        };

        if (!hasSuitablePresentationDevice(window)) {
            GTEST_SKIP() << "No suitable presentation-capable Vulkan device";
        }
    }

    Core::ApplicationConfiguration configuration{};
    configuration.application_name =
        "Signum Application External Rendering Test";
    configuration.initial_window_size = kWindowSize;
    configuration.development_diagnostics_enabled = false;

    Core::Application application{std::move(configuration)};

    const Vulkan::VulkanExternalRenderingInfo info =
        application.externalRenderingInfo();

    EXPECT_NE(info.instance, VK_NULL_HANDLE);
    EXPECT_NE(info.physical_device, VK_NULL_HANDLE);
    EXPECT_NE(info.device, VK_NULL_HANDLE);
    EXPECT_NE(info.graphics_queue, VK_NULL_HANDLE);

    EXPECT_EQ(info.api_version, Vulkan::kRequiredApiVersion);
    EXPECT_NE(info.color_attachment_format, VK_FORMAT_UNDEFINED);
    EXPECT_GT(info.swapchain_image_count, 0U);

    const Vulkan::VulkanExternalRenderingInfo second_snapshot =
        application.externalRenderingInfo();

    EXPECT_EQ(second_snapshot.instance, info.instance);
    EXPECT_EQ(second_snapshot.physical_device, info.physical_device);
    EXPECT_EQ(second_snapshot.device, info.device);
    EXPECT_EQ(second_snapshot.graphics_queue, info.graphics_queue);
    EXPECT_EQ(second_snapshot.graphics_queue_family_index,
              info.graphics_queue_family_index);
    EXPECT_EQ(second_snapshot.api_version, info.api_version);
    EXPECT_EQ(second_snapshot.color_attachment_format,
              info.color_attachment_format);
    EXPECT_EQ(second_snapshot.swapchain_image_count,
              info.swapchain_image_count);
}
