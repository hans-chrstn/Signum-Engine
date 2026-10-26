#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_discovery.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_selection.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_surface.hpp"
#include "engine/renderer/vulkan/vulkan_instance.hpp"
#include "engine/renderer/vulkan/vulkan_renderer.hpp"
#include "imgui/editor_imgui_context.hpp"
#include "imgui/editor_imgui_vulkan_backend.hpp"
#include <cstdlib>
#include <gtest/gtest.h>
#include <imgui.h>
#include <optional>

namespace Editor = SNE::Editor;
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
} // namespace

TEST(EditorImGuiVulkanBackendIntegrationTests, RejectsMissingImGuiContext) {
    ASSERT_EQ(ImGui::GetCurrentContext(), nullptr);

    const Vulkan::VulkanExternalRenderingInfo info{};

    EXPECT_DEATH(static_cast<void>(Editor::EditorImGuiVulkanBackend{info}),
                 "requires an active Dear ImGui context");
}

TEST(EditorImGuiVulkanBackendIntegrationTests,
     InitializesAndShutsDownRendererBackend) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw_context{};

    Platform::Window window{
        kWindowSize,
        "Signum ImGui Vulkan Backend Integration Test",
    };

    {
        Vulkan::VulkanInstance instance{
            "Signum ImGui Vulkan Backend Integration Test",
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
    }

    Vulkan::VulkanRenderer renderer{
        "Signum ImGui Vulkan Backend Integration Test",
        window,
        Renderer::PresentationPreference::VSync,
        false,
    };

    {
        Editor::EditorImGuiContext context{window.nativeHandle()};

        ASSERT_NE(ImGui::GetCurrentContext(), nullptr);
        ASSERT_EQ(ImGui::GetIO().BackendRendererUserData, nullptr);

        {
            std::optional<Editor::EditorImGuiVulkanBackend> backend;

            backend.emplace(renderer.externalRenderingInfo());

            ASSERT_NE(ImGui::GetIO().BackendRendererUserData, nullptr);
            EXPECT_NE(ImGui::GetIO().BackendRendererName, nullptr);

            backend.reset();

            EXPECT_EQ(ImGui::GetIO().BackendRendererUserData, nullptr);
            EXPECT_EQ(ImGui::GetIO().BackendRendererName, nullptr);

            backend.emplace(renderer.externalRenderingInfo());

            EXPECT_NE(ImGui::GetIO().BackendRendererUserData, nullptr);
            EXPECT_NE(ImGui::GetIO().BackendRendererName, nullptr);

            backend.reset();

            EXPECT_EQ(ImGui::GetIO().BackendRendererUserData, nullptr);
            EXPECT_EQ(ImGui::GetIO().BackendRendererName, nullptr);
        }

        EXPECT_EQ(ImGui::GetIO().BackendRendererUserData, nullptr);
        EXPECT_EQ(ImGui::GetIO().BackendRendererName, nullptr);
    }

    EXPECT_EQ(ImGui::GetCurrentContext(), nullptr);
}
