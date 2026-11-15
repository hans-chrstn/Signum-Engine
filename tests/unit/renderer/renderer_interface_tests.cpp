#include "engine/renderer/graphics_backend.hpp"
#include "engine/renderer/renderer_interface.hpp"
#include "engine/renderer/vulkan/vulkan_renderer.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <type_traits>
#include <utility>

namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    struct TestRendererState {
        bool queried{false};
        bool destroyed{false};
    };

    class TestRenderer final : public Renderer::IRenderer {
      private:
        TestRendererState &m_State;

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

    static_assert(
        std::is_same_v<decltype(std::declval<const Renderer::IRenderer &>()
                                    .graphicsBackend()),
                       Renderer::GraphicsBackend>);

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
