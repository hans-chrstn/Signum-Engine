#include "engine/renderer/resource/gpu_resource_handle.hpp"
#include <gtest/gtest.h>

namespace Renderer = SNE::Engine::Renderer;

TEST(GpuResourceHandleTests, DefaultHandleHasInvalidIdentity) {
    constexpr Renderer::GpuResourceHandle handle{};

    static_assert(handle.generation == 0U);
    static_assert(handle.registry_id == 0U);

    EXPECT_EQ(handle.index, 0U);
    EXPECT_EQ(handle.generation, 0U);
    EXPECT_EQ(handle.registry_id, 0U);
}

TEST(GpuResourceHandleTests, ComparesRegistryIndexAndGeneration) {
    constexpr Renderer::GpuResourceHandle original{
        .index = 5U,
        .generation = 1U,
        .registry_id = 7U,
    };

    constexpr Renderer::GpuResourceHandle equivalent{
        .index = 5U,
        .generation = 1U,
        .registry_id = 7U,
    };

    constexpr Renderer::GpuResourceHandle different_index{
        .index = 6U,
        .generation = 1U,
        .registry_id = 7U,
    };

    constexpr Renderer::GpuResourceHandle different_generation{
        .index = 5U,
        .generation = 2U,
        .registry_id = 7U,
    };

    constexpr Renderer::GpuResourceHandle different_registry{
        .index = 5U,
        .generation = 1U,
        .registry_id = 8U,
    };

    static_assert(original == equivalent);

    EXPECT_TRUE(original == equivalent);
    EXPECT_FALSE(original == different_index);
    EXPECT_FALSE(original == different_generation);
    EXPECT_FALSE(original == different_registry);
}
