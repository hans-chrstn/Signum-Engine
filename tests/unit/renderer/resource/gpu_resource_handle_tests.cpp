#include "engine/renderer/resource/gpu_resource_handle.hpp"
#include <gtest/gtest.h>

namespace Renderer = SNE::Engine::Renderer;

TEST(GpuResourceHandleTests, DefaultHandleHasInvalidGeneration) {
    constexpr Renderer::GpuResourceHandle handle{};

    static_assert(handle.generation == 0U);

    EXPECT_EQ(handle.index, 0U);
    EXPECT_EQ(handle.generation, 0U);
}

TEST(GpuResourceHandleTests, ComparesIndexAndGeneration) {
    constexpr Renderer::GpuResourceHandle original{
        .index = 5U,
        .generation = 1U,
    };

    constexpr Renderer::GpuResourceHandle equivalent{
        .index = 5U,
        .generation = 1U,
    };

    constexpr Renderer::GpuResourceHandle different_index{
        .index = 6U,
        .generation = 1U,
    };

    constexpr Renderer::GpuResourceHandle different_generation{
        .index = 5U,
        .generation = 2U,
    };

    static_assert(original == equivalent);

    EXPECT_TRUE(original == equivalent);
    EXPECT_FALSE(original == different_index);
    EXPECT_FALSE(original == different_generation);
}
