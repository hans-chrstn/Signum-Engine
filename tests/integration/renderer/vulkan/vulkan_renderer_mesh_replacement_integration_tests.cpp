#include "engine/core/math/matrix4.hpp"
#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/geometry/initial_mesh_data.hpp"
#include "engine/renderer/geometry/mesh_draw_instance.hpp"
#include "engine/renderer/geometry/vertex_attribute_description.hpp"
#include "engine/renderer/geometry/vertex_format.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/shader/graphics_shader_data.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_discovery.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_selection.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_spirv.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_surface.hpp"
#include "engine/renderer/vulkan/vulkan_instance.hpp"
#include "engine/renderer/vulkan/vulkan_renderer.hpp"
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <gtest/gtest.h>
#include <limits>
#include <numbers>
#include <optional>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

namespace Math = SNE::Engine::Core::Math;
namespace Platform = SNE::Engine::Platform;
namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;
namespace Shader = SNE::Engine::Renderer::Shader;

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

    constexpr std::size_t kSphereLatitudeSegments{10U};
    constexpr std::size_t kSphereLongitudeSegments{16U};
    constexpr std::size_t kSphereLongitudeVertexCount{
        kSphereLongitudeSegments + 1U,
    };

    constexpr std::size_t kSphereVertexCount{
        (kSphereLatitudeSegments + 1U) * kSphereLongitudeVertexCount,
    };

    constexpr float kTwo{2.0F};
    constexpr float kPi{std::numbers::pi_v<float>};
    constexpr float kFullRotation{kTwo * kPi};

    static_assert(kSphereVertexCount <=
                  std::numeric_limits<std::uint16_t>::max());

    struct ReferenceSphereGeometry {
        std::vector<TestVertex> vertices;
        std::vector<std::uint16_t> indices;
    };

    [[nodiscard]] auto makeReferenceSphereGeometry()
        -> ReferenceSphereGeometry {
        ReferenceSphereGeometry geometry{};

        geometry.vertices.reserve(kSphereVertexCount);

        for (std::size_t latitude{}; latitude <= kSphereLatitudeSegments;
             ++latitude) {
            const float latitude_angle =
                kPi * static_cast<float>(latitude) /
                static_cast<float>(kSphereLatitudeSegments);

            for (std::size_t longitude{}; longitude <= kSphereLongitudeSegments;
                 ++longitude) {
                const float longitude_angle =
                    kFullRotation * static_cast<float>(longitude) /
                    static_cast<float>(kSphereLongitudeSegments);

                geometry.vertices.push_back(TestVertex{
                    .position =
                        {
                            std::sin(latitude_angle) *
                                std::cos(longitude_angle),
                            std::cos(latitude_angle),
                            std::sin(latitude_angle) *
                                std::sin(longitude_angle),
                        },
                    .color = kGreen,
                });
            }
        }

        for (std::size_t latitude{}; latitude < kSphereLatitudeSegments;
             ++latitude) {
            for (std::size_t longitude{}; longitude < kSphereLongitudeSegments;
                 ++longitude) {
                const std::size_t upper_left_index =
                    (latitude * kSphereLongitudeVertexCount) + longitude;

                const std::size_t upper_right_index = upper_left_index + 1U;

                const std::size_t lower_left_index =
                    upper_left_index + kSphereLongitudeVertexCount;

                const std::size_t lower_right_index = lower_left_index + 1U;

                const auto upper_left =
                    static_cast<std::uint16_t>(upper_left_index);

                const auto upper_right =
                    static_cast<std::uint16_t>(upper_right_index);

                const auto lower_left =
                    static_cast<std::uint16_t>(lower_left_index);

                const auto lower_right =
                    static_cast<std::uint16_t>(lower_right_index);

                geometry.indices.insert(geometry.indices.end(), {
                                                                    upper_left,
                                                                    lower_left,
                                                                    upper_right,
                                                                    upper_right,
                                                                    lower_left,
                                                                    lower_right,
                                                                });
            }
        }

        return geometry;
    }
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
        Math::translate(-0.5F, -0.0F, 0.0F),
        Math::translate(0.5F, 0.0F, 0.0F),
    };

    const Renderer::InitialMeshData initial_mesh{
        .mesh = triangle_mesh,
        .layout =
            {
                .attributes = kVertexAttributes,
            },
        .model_matrices = model_matrices,
    };

    Shader::GraphicsShaderData graphics_data{
        .vertex =
            {
                .artifact = Vulkan::loadSpirv(
                    std::filesystem::path{SIGNUM_TEST_SHADER_DIR} /
                    "triangle.vert.spv"),
                .selection =
                    {
                        .stage = Shader::ShaderStage::Vertex,
                        .entry_point = "main",
                    },
            },
        .fragment =
            {
                .artifact = Vulkan::loadSpirv(
                    std::filesystem::path{SIGNUM_TEST_SHADER_DIR} /
                    "triangle.frag.spv"),
                .selection =
                    {
                        .stage = Shader::ShaderStage::Fragment,
                        .entry_point = "main",
                    },
            },
    };

    Vulkan::VulkanRenderer renderer{
        "Signum Mesh Replacement Test",
        window,
        Renderer::PresentationPreference::VSync,
        false,
        initial_mesh,
        std::nullopt,
        std::nullopt,
        std::move(graphics_data),
    };

    for (std::size_t frame{}; frame < kFramesPerMesh; ++frame) {
        ASSERT_NO_THROW(renderer.renderFrame());
    }

    ASSERT_NO_THROW(renderer.replaceMesh(quad_mesh));
    ASSERT_NO_THROW(renderer.replaceMesh(triangle_mesh));
    ASSERT_NO_THROW(renderer.replaceMesh(quad_mesh));

    for (std::size_t frame{}; frame < kFramesPerMesh; ++frame) {
        ASSERT_NO_THROW(renderer.renderFrame());
    }

    ASSERT_NO_THROW(renderer.replaceMesh(triangle_mesh));

    for (std::size_t frame{}; frame < kFramesPerMesh; ++frame) {
        ASSERT_NO_THROW(renderer.renderFrame());
    }
}

TEST(VulkanRendererMeshReplacementIntegrationTests,
     RendersIndependentlyRegisteredMeshesInSameFrame) {
    if (std::getenv("DISPLAY") == nullptr &&
        std::getenv("WAYLAND_DISPLAY") == nullptr) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw{};

    const Platform::Window window{
        kWindowSize,
        "Signum Independent Mesh Draw Test",
    };

    Vulkan::VulkanInstance instance{
        "Signum Independent Mesh Draw Test",
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

    const std::array initial_model_matrices{
        Math::identity(),
    };

    const Renderer::InitialMeshData initial_mesh{
        .mesh = triangle_mesh,
        .layout =
            {
                .attributes = kVertexAttributes,
            },
        .model_matrices = initial_model_matrices,
    };

    Shader::GraphicsShaderData graphics_data{
        .vertex =
            {
                .artifact = Vulkan::loadSpirv(
                    std::filesystem::path{SIGNUM_TEST_SHADER_DIR} /
                    "triangle.vert.spv"),
                .selection =
                    {
                        .stage = Shader::ShaderStage::Vertex,
                        .entry_point = "main",
                    },
            },
        .fragment =
            {
                .artifact = Vulkan::loadSpirv(
                    std::filesystem::path{SIGNUM_TEST_SHADER_DIR} /
                    "triangle.frag.spv"),
                .selection =
                    {
                        .stage = Shader::ShaderStage::Fragment,
                        .entry_point = "main",
                    },
            },
    };

    Vulkan::VulkanRenderer renderer{
        "Signum Independent Mesh Draw Test",
        window,
        Renderer::PresentationPreference::VSync,
        false,
        initial_mesh,
        std::nullopt,
        std::nullopt,
        std::move(graphics_data),
    };

    const auto triangle_handle = renderer.registerMesh(triangle_mesh);
    const auto quad_handle = renderer.registerMesh(quad_mesh);

    const std::array<Renderer::MeshDrawInstance, 3U> instances{
        Renderer::MeshDrawInstance{
            .mesh_handle = triangle_handle,
            .model_matrix = Math::translate(-0.5F, 0.0F, 0.0F),
            .material_handle = {},
        },
        Renderer::MeshDrawInstance{
            .mesh_handle = quad_handle,
            .model_matrix = Math::translate(0.5F, 0.0F, 0.0F),
            .material_handle = {},
        },
        Renderer::MeshDrawInstance{
            .mesh_handle = triangle_handle,
            .model_matrix = Math::translate(0.0F, 0.5F, 0.0F),
            .material_handle = {},
        },
    };

    ASSERT_NO_THROW(renderer.setDrawInstances(instances));

    for (std::size_t frame{}; frame < kFramesPerMesh; ++frame) {
        ASSERT_NO_THROW(renderer.renderFrame());
    }
}

TEST(VulkanRendererMeshReplacementIntegrationTests,
     UnregistersIndependentMeshesAndRejectsReferencedOrStaleHandles) {
    if (std::getenv("DISPLAY") == nullptr &&
        std::getenv("WAYLAND_DISPLAY") == nullptr) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw{};

    const Platform::Window window{
        kWindowSize,
        "Signum Mesh Unregistration Test",
    };

    Vulkan::VulkanInstance instance{
        "Signum Mesh Unregistration Test",
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

    const std::array initial_model_matrices{
        Math::identity(),
    };

    const Renderer::InitialMeshData initial_mesh{
        .mesh = triangle_mesh,
        .layout =
            {
                .attributes = kVertexAttributes,
            },
        .model_matrices = initial_model_matrices,
    };

    Shader::GraphicsShaderData graphics_data{
        .vertex =
            {
                .artifact = Vulkan::loadSpirv(
                    std::filesystem::path{SIGNUM_TEST_SHADER_DIR} /
                    "triangle.vert.spv"),
                .selection =
                    {
                        .stage = Shader::ShaderStage::Vertex,
                        .entry_point = "main",
                    },
            },
        .fragment =
            {
                .artifact = Vulkan::loadSpirv(
                    std::filesystem::path{SIGNUM_TEST_SHADER_DIR} /
                    "triangle.frag.spv"),
                .selection =
                    {
                        .stage = Shader::ShaderStage::Fragment,
                        .entry_point = "main",
                    },
            },
    };

    Vulkan::VulkanRenderer renderer{
        "Signum Mesh Unregistration Test",
        window,
        Renderer::PresentationPreference::VSync,
        false,
        initial_mesh,
        std::nullopt,
        std::nullopt,
        std::move(graphics_data),
    };

    const Renderer::GpuResourceHandle triangle_handle =
        renderer.registerMesh(triangle_mesh);

    const Renderer::GpuResourceHandle quad_handle =
        renderer.registerMesh(quad_mesh);

    const std::array initial_instances{
        Renderer::MeshDrawInstance{
            .mesh_handle = triangle_handle,
            .model_matrix = Math::translate(-0.5F, 0.0F, 0.0F),
            .material_handle = {},
        },
        Renderer::MeshDrawInstance{
            .mesh_handle = quad_handle,
            .model_matrix = Math::translate(0.5F, 0.0F, 0.0F),
            .material_handle = {},
        },
    };

    ASSERT_NO_THROW(renderer.setDrawInstances(initial_instances));

    for (std::size_t frame{}; frame < kFramesPerMesh; ++frame) {
        ASSERT_NO_THROW(renderer.renderFrame());
    }

    ASSERT_DEATH(
        { renderer.unregisterMesh(triangle_handle); },
        "cannot remove a referenced mesh");

    ASSERT_DEATH(
        { renderer.unregisterMesh(quad_handle); },
        "cannot remove a referenced mesh");

    const std::array remaining_instances{
        Renderer::MeshDrawInstance{
            .mesh_handle = quad_handle,
            .model_matrix = Math::identity(),
            .material_handle = {},
        },
    };

    ASSERT_NO_THROW(renderer.setDrawInstances(remaining_instances));

    ASSERT_NO_THROW(renderer.unregisterMesh(triangle_handle));

    ASSERT_DEATH(
        { renderer.unregisterMesh(triangle_handle); },
        "requires a registered mesh handle");

    const std::array stale_instances{
        Renderer::MeshDrawInstance{
            .mesh_handle = triangle_handle,
            .model_matrix = Math::identity(),
            .material_handle = {},
        },
    };

    ASSERT_DEATH(
        { renderer.setDrawInstances(stale_instances); },
        "requires registered mesh handles");

    const Renderer::GpuResourceHandle replacement_triangle_handle =
        renderer.registerMesh(triangle_mesh);

    EXPECT_EQ(replacement_triangle_handle.registry_id,
              triangle_handle.registry_id);

    EXPECT_EQ(replacement_triangle_handle.index, triangle_handle.index);

    EXPECT_EQ(replacement_triangle_handle.generation,
              triangle_handle.generation + 1U);

    const std::array replacement_instances{
        Renderer::MeshDrawInstance{
            .mesh_handle = replacement_triangle_handle,
            .model_matrix = Math::translate(-0.5F, 0.0F, 0.0F),
            .material_handle = {},
        },
        Renderer::MeshDrawInstance{
            .mesh_handle = quad_handle,
            .model_matrix = Math::translate(0.5F, 0.0F, 0.0F),
            .material_handle = {},
        },
    };

    ASSERT_NO_THROW(renderer.setDrawInstances(replacement_instances));

    for (std::size_t frame{}; frame < kFramesPerMesh; ++frame) {
        ASSERT_NO_THROW(renderer.renderFrame());
    }

    ASSERT_NO_THROW(renderer.setDrawInstances({}));

    ASSERT_NO_THROW(renderer.unregisterMesh(replacement_triangle_handle));
    ASSERT_NO_THROW(renderer.unregisterMesh(quad_handle));

    for (std::size_t frame{}; frame < kFramesPerMesh; ++frame) {
        ASSERT_NO_THROW(renderer.renderFrame());
    }
}

TEST(VulkanRendererMeshReplacementIntegrationTests,
     RendersSphereReferenceMeshAcrossIncreasingInstanceCounts) {
    constexpr std::size_t kSmallGridSide{4U};
    constexpr std::size_t kMediumGridSide{8U};
    constexpr std::size_t kLargeGridSide{12U};
    constexpr std::size_t kMeshAlternationPeriod{2U};
    constexpr std::size_t kFramesPerStage{3U};

    constexpr float kInstanceSpacing{0.14F};
    constexpr float kInstanceScale{0.065F};

    if (std::getenv("DISPLAY") == nullptr &&
        std::getenv("WAYLAND_DISPLAY") == nullptr) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw{};

    const Platform::Window window{
        kWindowSize,
        "Signum Sphere Reference Stress Test",
    };

    Vulkan::VulkanInstance instance{
        "Signum Sphere Reference Stress Test",
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

    const ReferenceSphereGeometry sphere = makeReferenceSphereGeometry();

    ASSERT_EQ(sphere.vertices.size(), kSphereVertexCount);
    ASSERT_FALSE(sphere.indices.empty());

    const Renderer::MeshUploadData sphere_mesh{
        .vertex_bytes = std::as_bytes(std::span{sphere.vertices}),
        .vertex_stride = kVertexStride,
        .indices = sphere.indices,
    };

    const Renderer::MeshUploadData quad_mesh{
        .vertex_bytes = std::as_bytes(std::span{kQuadVertices}),
        .vertex_stride = kVertexStride,
        .indices = kQuadIndices,
    };

    const Renderer::MeshUploadData initial_triangle_mesh{
        .vertex_bytes = std::as_bytes(std::span{kTriangleVertices}),
        .vertex_stride = kVertexStride,
        .indices = kTriangleIndices,
    };

    const std::array initial_model_matrices{
        Math::identity(),
    };

    const Renderer::InitialMeshData initial_mesh{
        .mesh = initial_triangle_mesh,
        .layout =
            {
                .attributes = kVertexAttributes,
            },
        .model_matrices = initial_model_matrices,
    };

    const std::filesystem::path shader_directory{
        SIGNUM_TEST_SHADER_DIR,
    };

    Shader::GraphicsShaderData graphics_data{
        .vertex =
            {
                .artifact =
                    Vulkan::loadSpirv(shader_directory / "triangle.vert.spv"),
                .selection =
                    {
                        .stage = Shader::ShaderStage::Vertex,
                        .entry_point = "main",
                    },
            },
        .fragment =
            {
                .artifact =
                    Vulkan::loadSpirv(shader_directory / "triangle.frag.spv"),
                .selection =
                    {
                        .stage = Shader::ShaderStage::Fragment,
                        .entry_point = "main",
                    },
            },
    };

    Vulkan::VulkanRenderer renderer{
        "Signum Sphere Reference Stress Test",
        window,
        Renderer::PresentationPreference::VSync,
        false,
        initial_mesh,
        std::nullopt,
        std::nullopt,
        std::move(graphics_data),
    };

    const Renderer::GpuResourceHandle sphere_handle =
        renderer.registerMesh(sphere_mesh);

    const Renderer::GpuResourceHandle quad_handle =
        renderer.registerMesh(quad_mesh);

    const auto make_grid =
        [&](std::size_t grid_side) -> std::vector<Renderer::MeshDrawInstance> {
        std::vector<Renderer::MeshDrawInstance> instances{};

        instances.reserve(grid_side * grid_side);

        for (std::size_t row{}; row < grid_side; ++row) {
            for (std::size_t column{}; column < grid_side; ++column) {
                const float x_position =
                    (static_cast<float>(column) -
                     (static_cast<float>(grid_side - 1U) * kHalfExtent)) *
                    kInstanceSpacing;

                const float y_position =
                    (static_cast<float>(row) -
                     (static_cast<float>(grid_side - 1U) * kHalfExtent)) *
                    kInstanceSpacing;

                const Renderer::GpuResourceHandle mesh_handle =
                    ((row + column) % kMeshAlternationPeriod) == 0U
                        ? sphere_handle
                        : quad_handle;

                const Math::Matrix4f model_matrix = Math::multiply(
                    Math::translate(x_position, y_position, 0.0F),
                    Math::scale(kInstanceScale, kInstanceScale,
                                kInstanceScale));

                instances.push_back(Renderer::MeshDrawInstance{
                    .mesh_handle = mesh_handle,
                    .model_matrix = model_matrix,
                    .material_handle = {},
                });
            }
        }

        return instances;
    };

    const std::array grid_sizes{
        kSmallGridSide,
        kMediumGridSide,
        kLargeGridSide,
        kSmallGridSide,
    };

    for (const std::size_t grid_side : grid_sizes) {
        const std::vector<Renderer::MeshDrawInstance> instances =
            make_grid(grid_side);

        ASSERT_EQ(instances.size(), grid_side * grid_side);

        ASSERT_NO_THROW(renderer.setDrawInstances(instances));

        for (std::size_t frame{}; frame < kFramesPerStage; ++frame) {
            ASSERT_NO_THROW(renderer.renderFrame());
        }
    }

    ASSERT_NO_THROW(renderer.setDrawInstances({}));

    ASSERT_NO_THROW(renderer.unregisterMesh(sphere_handle));
    ASSERT_NO_THROW(renderer.unregisterMesh(quad_handle));

    for (std::size_t frame{}; frame < kFramesPerStage; ++frame) {
        ASSERT_NO_THROW(renderer.renderFrame());
    }
}

TEST(VulkanRendererMeshReplacementIntegrationTests,
     UpdatesDrawInstanceModelMatricesAndRejectsInvalidIndices) {
    if (std::getenv("DISPLAY") == nullptr &&
        std::getenv("WAYLAND_DISPLAY") == nullptr) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw{};

    const Platform::Window window{
        kWindowSize,
        "Signum Draw Instance Transform Test",
    };

    Vulkan::VulkanInstance instance{
        "Signum Draw Instance Transform Test",
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

    const std::array initial_model_matrices{
        Math::identity(),
        Math::translate(0.5F, 0.0F, 0.0F),
    };

    const Renderer::InitialMeshData initial_mesh{
        .mesh = triangle_mesh,
        .layout =
            {
                .attributes = kVertexAttributes,
            },
        .model_matrices = initial_model_matrices,
    };

    const std::filesystem::path shader_directory{
        SIGNUM_TEST_SHADER_DIR,
    };

    Shader::GraphicsShaderData graphics_data{
        .vertex =
            {
                .artifact =
                    Vulkan::loadSpirv(shader_directory / "triangle.vert.spv"),
                .selection =
                    {
                        .stage = Shader::ShaderStage::Vertex,
                        .entry_point = "main",
                    },
            },
        .fragment =
            {
                .artifact =
                    Vulkan::loadSpirv(shader_directory / "triangle.frag.spv"),
                .selection =
                    {
                        .stage = Shader::ShaderStage::Fragment,
                        .entry_point = "main",
                    },
            },
    };

    Vulkan::VulkanRenderer renderer{
        "Signum Draw Instance Transform Test",
        window,
        Renderer::PresentationPreference::VSync,
        false,
        initial_mesh,
        std::nullopt,
        std::nullopt,
        std::move(graphics_data),
    };

    ASSERT_NO_THROW(renderer.renderFrame());

    const Math::Matrix4f first_transform = Math::multiply(
        Math::translate(-0.25F, 0.0F, 0.0F), Math::rotateY(0.5F));

    const Math::Matrix4f second_transform = Math::multiply(
        Math::translate(0.25F, 0.0F, 0.0F), Math::rotateX(0.75F));

    ASSERT_NO_THROW(renderer.setDrawInstanceModelMatrix(0U, first_transform));

    ASSERT_NO_THROW(renderer.setDrawInstanceModelMatrix(1U, second_transform));

    for (std::size_t frame{}; frame < kFramesPerMesh; ++frame) {
        ASSERT_NO_THROW(renderer.renderFrame());
    }

    ASSERT_NO_THROW(renderer.setDrawInstanceModelMatrix(0U, Math::identity()));

    ASSERT_NO_THROW(renderer.renderFrame());

    constexpr std::size_t kInvalidInstanceIndex{2U};

    ASSERT_DEATH(
        {
            renderer.setDrawInstanceModelMatrix(kInvalidInstanceIndex,
                                                Math::identity());
        },
        "requires a valid draw instance index");

    ASSERT_NO_THROW(renderer.setDrawInstances({}));

    ASSERT_DEATH(
        { renderer.setDrawInstanceModelMatrix(0U, Math::identity()); },
        "requires a valid draw instance index");

    ASSERT_NO_THROW(renderer.renderFrame());
}
