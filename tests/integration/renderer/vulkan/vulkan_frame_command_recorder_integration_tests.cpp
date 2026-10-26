#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_discovery.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_selection.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_surface.hpp"
#include "engine/renderer/vulkan/vulkan_frame_command_recorder.hpp"
#include "engine/renderer/vulkan/vulkan_instance.hpp"
#include "engine/renderer/vulkan/vulkan_renderer.hpp"
#include <cstddef>
#include <cstdlib>
#include <gtest/gtest.h>
#include <vulkan/vulkan.h>

namespace Platform = SNE::Engine::Platform;
namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = Renderer::Vulkan;

namespace {
    constexpr Platform::WindowSize kWindowSize{
        .width = 640,
        .height = 480,
    };

    constexpr std::size_t kFrameCount = 3U;
} // namespace

TEST(VulkanFrameCommandRecorderIntegrationTests,
     RecordsExternalCommandsWithoutMeshes) {
    if (std::getenv("DISPLAY") == nullptr &&
        std::getenv("WAYLAND_DISPLAY") == nullptr) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw_context{};

    const Platform::Window window{
        kWindowSize,
        "Signum Frame Command Recorder Integration Test",
    };

    Vulkan::VulkanInstance instance{
        "Signum Frame Command Recorder Integration Test",
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

    if (!Vulkan::selectPhysicalDevice(discovered).has_value()) {
        GTEST_SKIP() << "No suitable presentation-capable Vulkan device";
    }

    Vulkan::VulkanRenderer renderer{
        "Signum Frame Command Recorder Integration Test",
        window,
        Renderer::PresentationPreference::VSync,
        false,
    };

    ASSERT_NO_THROW(renderer.renderFrame());

    std::size_t invocation_count{};
    bool received_null_command_buffer = false;

    const Vulkan::VulkanFrameCommandRecorder recorder =
        [&](VkCommandBuffer command_buffer) -> void {
        ++invocation_count;

        if (command_buffer == VK_NULL_HANDLE) {
            received_null_command_buffer = true;
        }
    };

    for (std::size_t frame{}; frame < kFrameCount; ++frame) {
        ASSERT_NO_THROW(renderer.renderFrame(recorder));
    }

    EXPECT_EQ(invocation_count, kFrameCount);
    EXPECT_FALSE(received_null_command_buffer);

    ASSERT_NO_THROW(renderer.renderFrame());

    EXPECT_EQ(invocation_count, kFrameCount);
}
