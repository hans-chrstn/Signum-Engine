#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_discovery.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_selection.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_surface.hpp"
#include "engine/renderer/vulkan/vulkan_instance.hpp"
#include "engine/renderer/vulkan/vulkan_renderer.hpp"
#include <cstddef>
#include <cstdlib>
#include <gtest/gtest.h>

namespace Platform = SNE::Engine::Platform;
namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    constexpr Platform::WindowSize kWindowSize{
        .width = 640,
        .height = 480,
    };

    constexpr std::size_t kFrameCount{3U};
} // namespace

TEST(VulkanRendererMaterialIntegrationTests, RendersWithoutMeshOrMaterial) {
    if (std::getenv("DISPLAY") == nullptr &&
        std::getenv("WAYLAND_DISPLAY") == nullptr) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw{};

    const Platform::Window window{
        kWindowSize,
        "Signum Material Integration Test",
    };

    Vulkan::VulkanInstance instance{
        "Signum Material Integration Test",
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
        "Signum Material Integration Test",
        window,
        Renderer::PresentationPreference::VSync,
        false,
    };

    for (std::size_t frame{}; frame < kFrameCount; ++frame) {
        ASSERT_NO_THROW(renderer.renderFrame());
    }
}
