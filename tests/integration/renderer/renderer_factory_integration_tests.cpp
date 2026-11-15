#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/graphics_backend.hpp"
#include "engine/renderer/renderer_creation_info.hpp"
#include "engine/renderer/renderer_factory.hpp"
#include "engine/renderer/renderer_interface.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_discovery.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_selection.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_surface.hpp"
#include "engine/renderer/vulkan/vulkan_instance.hpp"
#include <bit>
#include <cstdint>
#include <cstdlib>
#include <gtest/gtest.h>
#include <limits>
#include <memory>
#include <utility>

namespace Platform = SNE::Engine::Platform;
namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    constexpr Platform::WindowSize kWindowSize{
        .width = 640,
        .height = 480,
    };

    constexpr std::uint8_t kUnrecognizedBackendValue =
        std::numeric_limits<std::uint8_t>::max();

    [[nodiscard]] auto hasDisplay() -> bool {
        return std::getenv("DISPLAY") != nullptr ||
               std::getenv("WAYLAND_DISPLAY") != nullptr;
    }

    [[nodiscard]] auto hasSuitableDevice(const Platform::Window &window)
        -> bool {
        Vulkan::VulkanInstance instance{
            "Signum Renderer Factory Integration Test",
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

TEST(RendererFactoryIntegrationTests, CreatesVulkanRendererThroughInterface) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw{};

    const Platform::Window window{
        kWindowSize,
        "Signum Renderer Factory Integration Test",
    };

    if (!hasSuitableDevice(window)) {
        GTEST_SKIP() << "No suitable presentation-capable Vulkan device";
    }

    Renderer::RendererCreationInfo creation_info{
        .backend = Renderer::GraphicsBackend::Vulkan,
        .application_name = "Signum Renderer Factory Integration Test",
        .window = window,
        .presentation_preference = Renderer::PresentationPreference::VSync,
        .development_diagnostics_enabled = false,
        .initial_mesh = std::nullopt,
        .initial_texture = std::nullopt,
        .initial_camera = std::nullopt,
        .initial_graphics_shaders = std::nullopt,
        .initial_buffer_bindings = {},
        .initial_texture_bindings = {},
        .require_swapchain_transfer_source = false,
        .logger = nullptr,
    };

    std::unique_ptr<Renderer::IRenderer> renderer =
        Renderer::createRenderer(std::move(creation_info));

    ASSERT_NE(renderer, nullptr);

    EXPECT_EQ(renderer->graphicsBackend(), Renderer::GraphicsBackend::Vulkan);
}

TEST(RendererFactoryIntegrationTests, RejectsUnrecognizedGraphicsBackend) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw{};

    const Platform::Window window{
        kWindowSize,
        "Signum Renderer Factory Integration Test",
    };

    Renderer::RendererCreationInfo creation_info{
        .backend =
            std::bit_cast<Renderer::GraphicsBackend>(kUnrecognizedBackendValue),
        .application_name = "Signum Renderer Factory Integration Test",
        .window = window,
        .presentation_preference = Renderer::PresentationPreference::VSync,
        .development_diagnostics_enabled = false,
        .initial_mesh = std::nullopt,
        .initial_texture = std::nullopt,
        .initial_camera = std::nullopt,
        .initial_graphics_shaders = std::nullopt,
        .initial_buffer_bindings = {},
        .initial_texture_bindings = {},
        .require_swapchain_transfer_source = false,
        .logger = nullptr,
    };

    ASSERT_DEATH(
        static_cast<void>(Renderer::createRenderer(std::move(creation_info))),
        "Renderer factory requires a recognized graphics backend");
}
