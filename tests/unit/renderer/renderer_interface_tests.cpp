#include "engine/core/math/matrix4.hpp"
#include "engine/renderer/geometry/vertex_attribute_description.hpp"
#include "engine/renderer/geometry/vertex_format.hpp"
#include "engine/renderer/graphics_backend.hpp"
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
    constexpr std::uint32_t kTestMeshIndex = 0U;
    constexpr std::uint32_t kTestMeshGeneration = 1U;
    constexpr std::uint64_t kTestMeshRegistryId = 1U;

    constexpr Renderer::GpuResourceHandle kTestRegisteredMeshHandle{
        .index = kTestMeshIndex,
        .generation = kTestMeshGeneration,
        .registry_id = kTestMeshRegistryId,
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

        std::size_t received_instance_count{0};
        std::size_t received_instance_index{0};
        std::size_t received_vertex_byte_count{0};
        std::size_t received_index_count{0};
        std::size_t received_attribute_count{0};

        std::uint32_t received_vertex_stride{0};

        Math::Matrix4f received_camera_view{};
        Math::Matrix4f received_model_matrix{};

        std::optional<Renderer::RenderTargetExtent> received_viewport_extent =
            std::nullopt;

        Renderer::GpuResourceHandle registered_mesh_handle{
            kTestRegisteredMeshHandle,
        };
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
                                    .drawInstances()),
                       std::vector<Renderer::MeshDrawInstance>>);

    static_assert(
        std::is_same_v<
            decltype(std::declval<Renderer::IRenderer &>().registerMesh(
                std::declval<const Renderer::MeshUploadData &>(),
                std::declval<const Renderer::VertexLayoutDescription &>())),
            Renderer::GpuResourceHandle>);

    static_assert(noexcept(
        std::declval<const Renderer::IRenderer &>().graphicsBackend()));

    static_assert(noexcept(
        std::declval<const Vulkan::VulkanRenderer &>().graphicsBackend()));
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
