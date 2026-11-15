#include "engine/core/math/matrix4.hpp"
#include "engine/renderer/geometry/vertex_attribute_description.hpp"
#include "engine/renderer/geometry/vertex_format.hpp"
#include "engine/renderer/graphics_backend.hpp"
#include "engine/renderer/graphics_submission_progress.hpp"
#include "engine/renderer/pipeline/cull_mode.hpp"
#include "engine/renderer/renderer_interface.hpp"
#include "engine/renderer/vulkan/vulkan_renderer.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>
#include <memory>
#include <optional>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

namespace Math = SNE::Engine::Core::Math;
namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    constexpr std::uint32_t kTestResourceIndex = 0U;
    constexpr std::uint32_t kTestResourceGeneration = 1U;

    constexpr std::uint64_t kTestMeshRegistryId = 1U;
    constexpr std::uint64_t kTestTextureRegistryId = 2U;
    constexpr std::uint64_t kTestMaterialConfigurationRegistryId = 3U;

    constexpr std::uint32_t kTextureWidth = 2U;
    constexpr std::uint32_t kTextureHeight = 1U;
    constexpr std::size_t kRgba8BytesPerPixel = 4U;

    constexpr std::size_t kTextureByteCount =
        static_cast<std::size_t>(kTextureWidth) *
        static_cast<std::size_t>(kTextureHeight) * kRgba8BytesPerPixel;

    constexpr Renderer::GpuResourceHandle kTestRegisteredMeshHandle{
        .index = kTestResourceIndex,
        .generation = kTestResourceGeneration,
        .registry_id = kTestMeshRegistryId,
    };

    constexpr Renderer::GpuResourceHandle kTestRegisteredTextureHandle{
        .index = kTestResourceIndex,
        .generation = kTestResourceGeneration,
        .registry_id = kTestTextureRegistryId,
    };

    constexpr Renderer::GpuResourceHandle
        kTestRegisteredMaterialConfigurationHandle{
            .index = kTestResourceIndex,
            .generation = kTestResourceGeneration,
            .registry_id = kTestMaterialConfigurationRegistryId,
    };

    struct TestRendererState {
        bool queried{false};
        bool destroyed{false};
        bool draw_instances_set{false};
        bool draw_instances_queried{false};
        bool camera_view_set{false};
        bool model_matrix_set{false};
        bool viewport_extent_set{false};
        bool mesh_registered{false};
        bool texture_registered{false};
        bool material_configuration_registered{false};
        bool material_configuration_unregistered{false};
        bool mesh_unregistered{false};
        bool submission_progress_queried{false};

        std::size_t received_instance_count{0};
        std::size_t received_instance_index{0};
        std::size_t received_vertex_byte_count{0};
        std::size_t received_index_count{0};
        std::size_t received_attribute_count{0};
        std::size_t received_texture_pixel_count{0};

        std::uint32_t received_vertex_stride{0};
        std::uint32_t received_texture_width{0};
        std::uint32_t received_texture_height{0};
        std::uint32_t received_texture_mip_levels{0};

        Renderer::TextureFormat received_texture_format{
            Renderer::TextureFormat::Rgba8Unorm,
        };

        Math::Matrix4f received_camera_view{};
        Math::Matrix4f received_model_matrix{};

        Renderer::GraphicsSubmissionProgress submission_progress{};

        std::optional<Renderer::RenderTargetExtent> received_viewport_extent =
            std::nullopt;

        std::optional<Renderer::MaterialDescription>
            received_material_configuration = std::nullopt;

        Renderer::GpuResourceHandle registered_mesh_handle{
            kTestRegisteredMeshHandle,
        };

        Renderer::GpuResourceHandle registered_texture_handle{
            kTestRegisteredTextureHandle,
        };

        Renderer::GpuResourceHandle registered_material_configuration_handle{
            kTestRegisteredMaterialConfigurationHandle,
        };

        Renderer::GpuResourceHandle
            unregistered_material_configuration_handle{};
        Renderer::GpuResourceHandle unregistered_mesh_handle{};
    };

    class TestRenderer final : public Renderer::IRenderer {
      private:
        TestRendererState &m_State;
        std::vector<Renderer::MeshDrawInstance> m_DrawInstances;

      public:
        explicit TestRenderer(TestRendererState &state) noexcept
            : m_State(state) {}

        ~TestRenderer() noexcept override {
            m_State.destroyed = true;
        }

        [[nodiscard]] auto graphicsBackend() const noexcept
            -> Renderer::GraphicsBackend override {
            m_State.queried = true;
            return Renderer::GraphicsBackend::Vulkan;
        }

        [[nodiscard]] auto graphicsSubmissionProgress() const noexcept
            -> Renderer::GraphicsSubmissionProgress override {
            m_State.submission_progress_queried = true;
            return m_State.submission_progress;
        }

        auto
        setDrawInstances(std::span<const Renderer::MeshDrawInstance> instances)
            -> void override {
            std::vector<Renderer::MeshDrawInstance> replacement(
                instances.begin(), instances.end());

            m_DrawInstances.swap(replacement);

            m_State.draw_instances_set = true;
            m_State.received_instance_count = instances.size();
        }

        [[nodiscard]] auto drawInstances() const
            -> std::vector<Renderer::MeshDrawInstance> override {
            m_State.draw_instances_queried = true;
            return m_DrawInstances;
        }

        auto setCameraViewMatrix(const Math::Matrix4f &view_matrix)
            -> void override {
            m_State.camera_view_set = true;
            m_State.received_camera_view = view_matrix;
        }

        auto setDrawInstanceModelMatrix(std::size_t instance_index,
                                        const Math::Matrix4f &model_matrix)
            -> void override {
            m_State.model_matrix_set = true;
            m_State.received_instance_index = instance_index;
            m_State.received_model_matrix = model_matrix;

            m_DrawInstances.at(instance_index).model_matrix = model_matrix;
        }

        auto setSceneViewportExtent(
            std::optional<Renderer::RenderTargetExtent> extent)
            -> void override {
            m_State.viewport_extent_set = true;
            m_State.received_viewport_extent = extent;
        }

        [[nodiscard]] auto
        registerMesh(const Renderer::MeshUploadData &mesh,
                     const Renderer::VertexLayoutDescription &layout)
            -> Renderer::GpuResourceHandle override {
            m_State.mesh_registered = true;
            m_State.received_vertex_byte_count = mesh.vertex_bytes.size();
            m_State.received_vertex_stride = mesh.vertex_stride;
            m_State.received_index_count = mesh.indices.size();
            m_State.received_attribute_count = layout.attributes.size();

            return m_State.registered_mesh_handle;
        }

        [[nodiscard]] auto
        registerTexture(const Renderer::TextureUploadData &texture)
            -> Renderer::GpuResourceHandle override {
            m_State.texture_registered = true;
            m_State.received_texture_width = texture.width;
            m_State.received_texture_height = texture.height;
            m_State.received_texture_pixel_count = texture.pixels.size();
            m_State.received_texture_format = texture.format;
            m_State.received_texture_mip_levels = texture.mip_levels;

            return m_State.registered_texture_handle;
        }

        [[nodiscard]] auto
        registerMaterialConfiguration(Renderer::MaterialDescription description)
            -> Renderer::GpuResourceHandle override {
            m_State.received_material_configuration.emplace(
                std::move(description));

            m_State.material_configuration_registered = true;

            return m_State.registered_material_configuration_handle;
        }

        auto unregisterMaterialConfiguration(Renderer::GpuResourceHandle handle)
            -> void override {
            m_State.material_configuration_unregistered = true;
            m_State.unregistered_material_configuration_handle = handle;
            m_State.received_material_configuration.reset();
        }

        auto unregisterMesh(Renderer::GpuResourceHandle handle)
            -> void override {
            m_State.mesh_unregistered = true;
            m_State.unregistered_mesh_handle = handle;
        }
    };

    static_assert(std::is_abstract_v<Renderer::IRenderer>);
    static_assert(std::has_virtual_destructor_v<Renderer::IRenderer>);

    static_assert(!std::is_copy_constructible_v<Renderer::IRenderer>);
    static_assert(!std::is_copy_assignable_v<Renderer::IRenderer>);
    static_assert(!std::is_move_constructible_v<Renderer::IRenderer>);
    static_assert(!std::is_move_assignable_v<Renderer::IRenderer>);

    static_assert(
        std::is_base_of_v<Renderer::IRenderer, Vulkan::VulkanRenderer>);

    static_assert(!std::is_abstract_v<Vulkan::VulkanRenderer>);
    static_assert(!std::is_abstract_v<TestRenderer>);

    static_assert(
        std::is_same_v<decltype(std::declval<const Renderer::IRenderer &>()
                                    .graphicsBackend()),
                       Renderer::GraphicsBackend>);

    static_assert(
        std::is_same_v<decltype(std::declval<const Renderer::IRenderer &>()
                                    .graphicsSubmissionProgress()),
                       Renderer::GraphicsSubmissionProgress>);

    static_assert(
        std::is_same_v<decltype(std::declval<const Vulkan::VulkanRenderer &>()
                                    .graphicsSubmissionProgress()),
                       Renderer::GraphicsSubmissionProgress>);

    static_assert(
        std::is_same_v<decltype(std::declval<const Renderer::IRenderer &>()
                                    .drawInstances()),
                       std::vector<Renderer::MeshDrawInstance>>);

    static_assert(
        std::is_same_v<
            decltype(std::declval<Renderer::IRenderer &>().registerMesh(
                std::declval<const Renderer::MeshUploadData &>(),
                std::declval<const Renderer::VertexLayoutDescription &>())),
            Renderer::GpuResourceHandle>);

    static_assert(
        std::is_same_v<
            decltype(std::declval<Renderer::IRenderer &>().registerTexture(
                std::declval<const Renderer::TextureUploadData &>())),
            Renderer::GpuResourceHandle>);

    static_assert(
        std::is_same_v<
            decltype(std::declval<Renderer::IRenderer &>()
                         .registerMaterialConfiguration(
                             std::declval<Renderer::MaterialDescription>())),
            Renderer::GpuResourceHandle>);

    static_assert(noexcept(
        std::declval<const Renderer::IRenderer &>().graphicsBackend()));

    static_assert(noexcept(
        std::declval<const Vulkan::VulkanRenderer &>().graphicsBackend()));

    static_assert(noexcept(std::declval<const Renderer::IRenderer &>()
                               .graphicsSubmissionProgress()));

    static_assert(noexcept(std::declval<const Vulkan::VulkanRenderer &>()
                               .graphicsSubmissionProgress()));
} // namespace

TEST(RendererInterfaceTests, DispatchesBackendQueryThroughBaseReference) {
    TestRendererState state{};

    {
        TestRenderer concrete_renderer{state};

        const Renderer::IRenderer &renderer = concrete_renderer;

        EXPECT_EQ(renderer.graphicsBackend(),
                  Renderer::GraphicsBackend::Vulkan);
        EXPECT_TRUE(state.queried);
        EXPECT_FALSE(state.destroyed);
    }

    EXPECT_TRUE(state.destroyed);
}

TEST(RendererInterfaceTests,
     DispatchesDrawInstanceUpdatesThroughBaseReference) {
    TestRendererState state{};

    {
        TestRenderer concrete_renderer{state};
        Renderer::IRenderer &renderer = concrete_renderer;

        const std::array<Renderer::MeshDrawInstance, 2> instances{};

        renderer.setDrawInstances(instances);

        EXPECT_TRUE(state.draw_instances_set);
        EXPECT_EQ(state.received_instance_count, instances.size());

        renderer.setDrawInstances({});

        EXPECT_EQ(state.received_instance_count, 0U);
        EXPECT_FALSE(state.destroyed);
    }

    EXPECT_TRUE(state.destroyed);
}

TEST(RendererInterfaceTests,
     ReturnsIndependentDrawInstanceSnapshotThroughBaseReference) {
    TestRendererState state{};

    {
        TestRenderer concrete_renderer{state};

        Renderer::IRenderer &renderer = concrete_renderer;
        const Renderer::IRenderer &const_renderer = concrete_renderer;

        const std::array<Renderer::MeshDrawInstance, 2> instances{};

        renderer.setDrawInstances(instances);

        const auto original_snapshot = const_renderer.drawInstances();

        EXPECT_TRUE(state.draw_instances_queried);
        ASSERT_EQ(original_snapshot.size(), instances.size());

        auto modified_snapshot = const_renderer.drawInstances();

        modified_snapshot.clear();

        EXPECT_TRUE(modified_snapshot.empty());

        const auto current_snapshot = const_renderer.drawInstances();

        EXPECT_EQ(current_snapshot.size(), instances.size());

        renderer.setDrawInstances({});

        EXPECT_TRUE(const_renderer.drawInstances().empty());
        EXPECT_EQ(original_snapshot.size(), instances.size());
        EXPECT_FALSE(state.destroyed);
    }

    EXPECT_TRUE(state.destroyed);
}

TEST(RendererInterfaceTests, DispatchesCameraViewUpdateThroughBaseReference) {
    TestRendererState state{};

    TestRenderer concrete_renderer{state};
    Renderer::IRenderer &renderer = concrete_renderer;

    const Math::Matrix4f view_matrix = Math::identity();

    renderer.setCameraViewMatrix(view_matrix);

    EXPECT_TRUE(state.camera_view_set);
    EXPECT_EQ(state.received_camera_view.elements, view_matrix.elements);
}

TEST(RendererInterfaceTests, DispatchesModelMatrixUpdateThroughBaseReference) {
    TestRendererState state{};

    TestRenderer concrete_renderer{state};
    Renderer::IRenderer &renderer = concrete_renderer;

    const std::array<Renderer::MeshDrawInstance, 2> instances{};
    renderer.setDrawInstances(instances);

    const Math::Matrix4f model_matrix = Math::identity();

    renderer.setDrawInstanceModelMatrix(1U, model_matrix);

    EXPECT_TRUE(state.model_matrix_set);
    EXPECT_EQ(state.received_instance_index, 1U);
    EXPECT_EQ(state.received_model_matrix.elements, model_matrix.elements);

    const auto snapshot = renderer.drawInstances();

    ASSERT_EQ(snapshot.size(), instances.size());
    EXPECT_EQ(snapshot[1].model_matrix.elements, model_matrix.elements);
}

TEST(RendererInterfaceTests,
     DispatchesViewportExtentUpdatesThroughBaseReference) {
    TestRendererState state{};

    TestRenderer concrete_renderer{state};
    Renderer::IRenderer &renderer = concrete_renderer;

    const Renderer::RenderTargetExtent extent{
        .width = 1280U,
        .height = 720U,
    };

    renderer.setSceneViewportExtent(extent);

    EXPECT_TRUE(state.viewport_extent_set);

    if (!state.received_viewport_extent.has_value()) {
        ADD_FAILURE() << "Renderer did not receive the viewport extent";
        return;
    }

    const Renderer::RenderTargetExtent received_extent =
        *state.received_viewport_extent;

    EXPECT_EQ(received_extent.width, extent.width);
    EXPECT_EQ(received_extent.height, extent.height);

    renderer.setSceneViewportExtent(std::nullopt);

    EXPECT_FALSE(state.received_viewport_extent.has_value());
}

TEST(RendererInterfaceTests, DispatchesMeshRegistrationThroughBaseReference) {
    TestRendererState state{};

    TestRenderer concrete_renderer{state};
    Renderer::IRenderer &renderer = concrete_renderer;

    const std::array<std::byte, 36> vertex_bytes{};
    const std::array<std::uint16_t, 3> indices{0U, 1U, 2U};

    const std::array<Renderer::VertexAttributeDescription, 1> attributes{
        Renderer::VertexAttributeDescription{
            .location = 0U,
            .format = Renderer::VertexFormat::Float32x3,
            .offset = 0U,
        },
    };

    const Renderer::MeshUploadData mesh{
        .vertex_bytes = vertex_bytes,
        .vertex_stride = 12U,
        .indices = indices,
    };

    const Renderer::VertexLayoutDescription layout{
        .attributes = attributes,
    };

    const Renderer::GpuResourceHandle handle =
        renderer.registerMesh(mesh, layout);

    EXPECT_TRUE(state.mesh_registered);
    EXPECT_EQ(state.received_vertex_byte_count, vertex_bytes.size());
    EXPECT_EQ(state.received_vertex_stride, mesh.vertex_stride);
    EXPECT_EQ(state.received_index_count, indices.size());
    EXPECT_EQ(state.received_attribute_count, attributes.size());
    EXPECT_EQ(handle, state.registered_mesh_handle);
}

TEST(RendererInterfaceTests,
     DispatchesTextureRegistrationThroughBaseReference) {
    constexpr std::uint32_t kTextureMipLevels = 1U;

    TestRendererState state{};

    TestRenderer concrete_renderer{state};
    Renderer::IRenderer &renderer = concrete_renderer;

    const std::array<std::byte, kTextureByteCount> pixels{};

    const Renderer::TextureUploadData texture{
        .width = kTextureWidth,
        .height = kTextureHeight,
        .pixels = pixels,
        .format = Renderer::TextureFormat::Rgba8Srgb,
        .mip_levels = kTextureMipLevels,
    };

    const Renderer::GpuResourceHandle handle =
        renderer.registerTexture(texture);

    EXPECT_TRUE(state.texture_registered);
    EXPECT_EQ(state.received_texture_width, texture.width);
    EXPECT_EQ(state.received_texture_height, texture.height);
    EXPECT_EQ(state.received_texture_pixel_count, pixels.size());
    EXPECT_EQ(state.received_texture_format, texture.format);
    EXPECT_EQ(state.received_texture_mip_levels, texture.mip_levels);
    EXPECT_EQ(handle, state.registered_texture_handle);
}

TEST(RendererInterfaceTests,
     DispatchesMaterialConfigurationRegistrationThroughBaseReference) {
    TestRendererState state{};

    TestRenderer concrete_renderer{state};
    Renderer::IRenderer &renderer = concrete_renderer;

    Renderer::MaterialDescription description{};
    description.pipeline_state.cull_mode = Renderer::CullMode::Back;

    const Renderer::GpuResourceHandle handle =
        renderer.registerMaterialConfiguration(std::move(description));

    EXPECT_TRUE(state.material_configuration_registered);
    EXPECT_EQ(handle, state.registered_material_configuration_handle);

    if (!state.received_material_configuration.has_value()) {
        ADD_FAILURE() << "Renderer did not retain the material configuration";
        return;
    }

    EXPECT_EQ(state.received_material_configuration->pipeline_state.cull_mode,
              Renderer::CullMode::Back);
}

TEST(RendererInterfaceTests,
     DispatchesMaterialConfigurationRemovalThroughBaseReference) {
    TestRendererState state{};

    TestRenderer concrete_renderer{state};
    Renderer::IRenderer &renderer = concrete_renderer;

    const Renderer::GpuResourceHandle handle =
        renderer.registerMaterialConfiguration(Renderer::MaterialDescription{});

    renderer.unregisterMaterialConfiguration(handle);

    EXPECT_TRUE(state.material_configuration_registered);
    EXPECT_TRUE(state.material_configuration_unregistered);
    EXPECT_EQ(state.unregistered_material_configuration_handle, handle);
    EXPECT_FALSE(state.received_material_configuration.has_value());
}

TEST(RendererInterfaceTests, DispatchesMeshRemovalThroughBaseReference) {
    TestRendererState state{};

    TestRenderer concrete_renderer{state};
    Renderer::IRenderer &renderer = concrete_renderer;

    renderer.unregisterMesh(kTestRegisteredMeshHandle);

    EXPECT_TRUE(state.mesh_unregistered);
    EXPECT_EQ(state.unregistered_mesh_handle, kTestRegisteredMeshHandle);
}

TEST(RendererInterfaceTests,
     ReturnsInitialGraphicsSubmissionProgressThroughBaseReference) {
    TestRendererState state{};

    TestRenderer concrete_renderer{state};
    const Renderer::IRenderer &renderer = concrete_renderer;

    const Renderer::GraphicsSubmissionProgress progress =
        renderer.graphicsSubmissionProgress();

    EXPECT_TRUE(state.submission_progress_queried);
    EXPECT_EQ(progress.last_submitted, 0U);
    EXPECT_EQ(progress.last_completed, 0U);
}

TEST(RendererInterfaceTests,
     ReturnsIndependentGraphicsSubmissionProgressSnapshotThroughBaseReference) {
    constexpr std::uint64_t kLastSubmittedSubmissionId = 5U;
    constexpr std::uint64_t kLastCompletedSubmissionId = 3U;
    constexpr std::uint64_t kNextSubmittedSubmissionId = 6U;

    TestRendererState state{};

    state.submission_progress = {
        .last_submitted = kLastSubmittedSubmissionId,
        .last_completed = kLastCompletedSubmissionId,
    };

    TestRenderer concrete_renderer{state};
    const Renderer::IRenderer &renderer = concrete_renderer;

    const Renderer::GraphicsSubmissionProgress original_snapshot =
        renderer.graphicsSubmissionProgress();

    EXPECT_TRUE(state.submission_progress_queried);
    EXPECT_EQ(original_snapshot.last_submitted, kLastSubmittedSubmissionId);
    EXPECT_EQ(original_snapshot.last_completed, kLastCompletedSubmissionId);

    state.submission_progress.last_submitted = kNextSubmittedSubmissionId;

    const Renderer::GraphicsSubmissionProgress updated_snapshot =
        renderer.graphicsSubmissionProgress();

    EXPECT_EQ(updated_snapshot.last_submitted, kNextSubmittedSubmissionId);
    EXPECT_EQ(updated_snapshot.last_completed, kLastCompletedSubmissionId);

    EXPECT_EQ(original_snapshot.last_submitted, kLastSubmittedSubmissionId);
    EXPECT_EQ(original_snapshot.last_completed, kLastCompletedSubmissionId);
}

TEST(RendererInterfaceTests, DestroysConcreteRendererThroughBasePointer) {
    TestRendererState state{};

    {
        std::unique_ptr<Renderer::IRenderer> renderer =
            std::make_unique<TestRenderer>(state);

        EXPECT_EQ(renderer->graphicsBackend(),
                  Renderer::GraphicsBackend::Vulkan);
        EXPECT_TRUE(state.queried);
        EXPECT_FALSE(state.destroyed);
    }

    EXPECT_TRUE(state.destroyed);
}
