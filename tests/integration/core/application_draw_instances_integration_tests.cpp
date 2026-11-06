#include "engine/core/application.hpp"
#include "engine/core/application_configuration.hpp"
#include "engine/core/math/matrix4.hpp"
#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/geometry/mesh_draw_instance.hpp"
#include "engine/renderer/geometry/mesh_upload_data.hpp"
#include "engine/renderer/geometry/vertex_attribute_description.hpp"
#include "engine/renderer/geometry/vertex_format.hpp"
#include "engine/renderer/geometry/vertex_layout_description.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_discovery.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_selection.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_surface.hpp"
#include "engine/renderer/vulkan/vulkan_instance.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <gtest/gtest.h>
#include <span>
#include <utility>

namespace Core = SNE::Engine::Core;
namespace Math = SNE::Engine::Core::Math;
namespace Platform = SNE::Engine::Platform;
namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    constexpr Platform::WindowSize kWindowSize{
        .width = 640,
        .height = 480,
    };

    constexpr float kHalfExtent{0.5F};
    constexpr float kZero{0.0F};
    constexpr float kInstanceTranslationX{1.0F};
    constexpr float kModifiedTranslationX{5.0F};

    constexpr std::uint32_t kPositionLocation{0U};

    struct TestVertex {
        std::array<float, 3> position;
    };

    constexpr std::array kVertices{
        TestVertex{
            .position = {-kHalfExtent, -kHalfExtent, kZero},
        },
        TestVertex{
            .position = {kHalfExtent, -kHalfExtent, kZero},
        },
        TestVertex{
            .position = {kZero, kHalfExtent, kZero},
        },
    };

    constexpr std::array<std::uint16_t, 3> kIndices{
        0U,
        1U,
        2U,
    };

    constexpr std::array kAttributes{
        Renderer::VertexAttributeDescription{
            .location = kPositionLocation,
            .format = Renderer::VertexFormat::Float32x3,
            .offset =
                static_cast<std::uint32_t>(offsetof(TestVertex, position)),
        },
    };

    [[nodiscard]] auto hasDisplay() -> bool {
        return std::getenv("DISPLAY") != nullptr ||
               std::getenv("WAYLAND_DISPLAY") != nullptr;
    }

    [[nodiscard]] auto hasSuitableDevice(const Platform::Window &window)
        -> bool {
        Vulkan::VulkanInstance instance{
            "Signum Application Draw Instance Test",
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

TEST(ApplicationDrawInstancesIntegrationTests,
     PreservesIndependentDrawInstanceSnapshots) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    {
        Platform::GlfwContext glfw{};

        const Platform::Window window{
            kWindowSize,
            "Signum Application Draw Instance Test",
        };

        if (!hasSuitableDevice(window)) {
            GTEST_SKIP() << "No suitable presentation-capable Vulkan device";
        }
    }

    Core::ApplicationConfiguration configuration{};
    configuration.application_name = "Signum Application Draw Instance Test";
    configuration.initial_window_size = kWindowSize;
    configuration.development_diagnostics_enabled = false;

    Core::Application application{std::move(configuration)};

    EXPECT_TRUE(application.drawInstances().empty());

    const Renderer::MeshUploadData mesh{
        .vertex_bytes = std::as_bytes(std::span{kVertices}),
        .vertex_stride = static_cast<std::uint32_t>(sizeof(TestVertex)),
        .indices = kIndices,
    };

    const Renderer::VertexLayoutDescription layout{
        .attributes = kAttributes,
    };

    const Renderer::GpuResourceHandle mesh_handle =
        application.registerMesh(mesh, layout);

    const Renderer::MeshDrawInstance first{
        .mesh_handle = mesh_handle,
        .model_matrix = Math::identity(),
    };

    const Renderer::MeshDrawInstance second{
        .mesh_handle = mesh_handle,
        .model_matrix = Math::translate(kInstanceTranslationX, kZero, kZero),
    };

    const std::array instances{first, second};

    application.setDrawInstances(instances);

    auto snapshot = application.drawInstances();

    ASSERT_EQ(snapshot.size(), instances.size());

    EXPECT_EQ(snapshot[0].mesh_handle, mesh_handle);
    EXPECT_EQ(snapshot[1].mesh_handle, mesh_handle);

    EXPECT_EQ(snapshot[0].model_matrix.elements, first.model_matrix.elements);

    EXPECT_EQ(snapshot[1].model_matrix.elements, second.model_matrix.elements);

    snapshot[0].mesh_handle = {};
    snapshot[1].model_matrix =
        Math::translate(kModifiedTranslationX, kZero, kZero);

    const auto unchanged = application.drawInstances();

    ASSERT_EQ(unchanged.size(), instances.size());

    EXPECT_EQ(unchanged[0].mesh_handle, mesh_handle);
    EXPECT_EQ(unchanged[1].mesh_handle, mesh_handle);

    EXPECT_EQ(unchanged[0].model_matrix.elements, first.model_matrix.elements);

    EXPECT_EQ(unchanged[1].model_matrix.elements, second.model_matrix.elements);

    application.setDrawInstances({});

    EXPECT_TRUE(application.drawInstances().empty());
}
