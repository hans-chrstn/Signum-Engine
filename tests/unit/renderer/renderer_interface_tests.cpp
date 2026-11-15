#include "engine/renderer/graphics_backend.hpp"
#include "engine/renderer/renderer_interface.hpp"
#include "engine/renderer/vulkan/vulkan_renderer.hpp"
#include <array>
#include <cstddef>
#include <gtest/gtest.h>
#include <memory>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    struct TestRendererState {
        bool queried{false};
        bool destroyed{false};
        bool draw_instances_set{false};
        bool draw_instances_queried{false};
        std::size_t received_instance_count{0};
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
