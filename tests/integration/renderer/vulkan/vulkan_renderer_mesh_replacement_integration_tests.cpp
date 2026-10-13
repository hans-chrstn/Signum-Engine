#include "engine/core/math/matrix4.hpp"
#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/geometry/initial_mesh_data.hpp"
#include "engine/renderer/geometry/vertex_attribute_description.hpp"
#include "engine/renderer/geometry/vertex_format.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_discovery.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_selection.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_surface.hpp"
#include "engine/renderer/vulkan/vulkan_instance.hpp"
#include "engine/renderer/vulkan/vulkan_renderer.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <gtest/gtest.h>
#include <span>
#include <type_traits>

namespace Math = SNE::Engine::Core::Math;
namespace Platform = SNE::Engine::Platform;
namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    struct TestVertex {
        std::array<float, 3> position;
        std::array<float, 3> color;
    };

    static_assert(std::is_standard_layout_v<TestVertex>);

    constexpr Platform::WindowSize kWindowSize{
        .width = 640,
        .height = 480,
    };

    constexpr float kHalfExtent = 0.5F;
    constexpr float kCenter = 0.0F;
    constexpr float kDepth = 0.0F;

    constexpr std::size_t kFramesPerMesh = 2U;

    constexpr std::uint32_t kVertexStride =
        static_cast<std::uint32_t>(sizeof(TestVertex));

    constexpr std::uint32_t kPositionLocation = 0U;
    constexpr std::uint32_t kColorLocation = 1U;

    constexpr std::array<float, 3> kRed{
        1.0F,
        0.0F,
        0.0F,
    };

    constexpr std::array<float, 3> kGreen{
        0.0F,
        1.0F,
        0.0F,
    };

    constexpr std::array<float, 3> kBlue{
        0.0F,
        0.0F,
        1.0F,
    };

    constexpr std::array<float, 3> kYellow{
        1.0F,
        1.0F,
        0.0F,
    };

    constexpr std::array kVertexAttributes{
        Renderer::VertexAttributeDescription{
            .location = kPositionLocation,
            .format = Renderer::VertexFormat::Float32x3,
            .offset =
                static_cast<std::uint32_t>(offsetof(TestVertex, position)),
        },
        Renderer::VertexAttributeDescription{
            .location = kColorLocation,
            .format = Renderer::VertexFormat::Float32x3,
            .offset = static_cast<std::uint32_t>(offsetof(TestVertex, color)),
        },
    };

    constexpr std::array kTriangleVertices{
        TestVertex{
            .position = {-kHalfExtent, -kHalfExtent, kDepth},
            .color = kRed,
        },
        TestVertex{
            .position = {kHalfExtent, -kHalfExtent, kDepth},
            .color = kGreen,
        },
        TestVertex{
            .position = {kCenter, kHalfExtent, kDepth},
            .color = kBlue,
        },
    };

    constexpr std::array<std::uint16_t, 3> kTriangleIndices{
        0U,
        1U,
        2U,
    };

    constexpr std::array kQuadVertices{
        TestVertex{
            .position = {-kHalfExtent, -kHalfExtent, kDepth},
            .color = kRed,
        },
        TestVertex{
            .position = {kHalfExtent, -kHalfExtent, kDepth},
            .color = kGreen,
        },
        TestVertex{
            .position = {-kHalfExtent, kHalfExtent, kDepth},
            .color = kBlue,
        },
        TestVertex{
            .position = {kHalfExtent, kHalfExtent, kDepth},
            .color = kYellow,
        },
    };

    constexpr std::array<std::uint16_t, 6> kQuadIndices{
        0U, 1U, 2U, 2U, 1U, 3U,
    };
} // namespace

TEST(VulkanRendererMeshReplacementIntegrationTests,
     ReplacesMeshAcrossRenderedFrames) {
    if (std::getenv("DISPLAY") == nullptr &&
        std::getenv("WAYLAND_DISPLAY") == nullptr) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw{};

    const Platform::Window window{
        kWindowSize,
        "Signum Mesh Replacement Test",
    };

    Vulkan::VulkanInstance instance{
        "Signum Mesh Replacement Test",
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

    const Renderer::MeshUploadData triangle_mesh{
        .vertex_bytes = std::as_bytes(std::span{kTriangleVertices}),
        .vertex_stride = kVertexStride,
        .indices = kTriangleIndices,
    };

    const Renderer::MeshUploadData quad_mesh{
        .vertex_bytes = std::as_bytes(std::span{kQuadVertices}),
        .vertex_stride = kVertexStride,
        .indices = kQuadIndices,
    };

    const std::array model_matrices{
        Math::identity(),
    };

    const Renderer::InitialMeshData initial_mesh{
        .mesh = triangle_mesh,
        .layout =
            {
                .attributes = kVertexAttributes,
            },
        .model_matrices = model_matrices,
    };

    Vulkan::VulkanRenderer renderer{
        "Signum Mesh Replacement Test",
        window,
        Renderer::PresentationPreference::VSync,
        false,
        initial_mesh,
    };

    for (std::size_t frame{}; frame < kFramesPerMesh; ++frame) {
        ASSERT_NO_THROW(renderer.renderFrame());
    }

    ASSERT_NO_THROW(renderer.replaceMesh(quad_mesh));

    for (std::size_t frame{}; frame < kFramesPerMesh; ++frame) {
        ASSERT_NO_THROW(renderer.renderFrame());
    }

    ASSERT_NO_THROW(renderer.replaceMesh(triangle_mesh));

    for (std::size_t frame{}; frame < kFramesPerMesh; ++frame) {
        ASSERT_NO_THROW(renderer.renderFrame());
    }
}
