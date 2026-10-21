
#include "engine/renderer/resource/gpu_resource_dependency.hpp"
#include <cstdint>
#include <gtest/gtest.h>

namespace Renderer = SNE::Engine::Renderer;

namespace {
    constexpr std::uint32_t kDependentIndex{2U};
    constexpr std::uint32_t kDependentGeneration{3U};
    constexpr std::uint64_t kDependentRegistryId{4U};

    constexpr std::uint32_t kDependencyIndex{5U};
    constexpr std::uint32_t kDependencyGeneration{6U};
    constexpr std::uint64_t kDependencyRegistryId{7U};

    constexpr Renderer::GpuResourceHandle kDependentHandle{
        .index = kDependentIndex,
        .generation = kDependentGeneration,
        .registry_id = kDependentRegistryId,
    };

    constexpr Renderer::GpuResourceHandle kDependencyHandle{
        .index = kDependencyIndex,
        .generation = kDependencyGeneration,
        .registry_id = kDependencyRegistryId,
    };
} // namespace

TEST(GpuResourceDependencyTests, DefaultHandlesAreInvalid) {
    constexpr Renderer::GpuResourceDependency relationship{};

    static_assert(relationship.dependent == Renderer::GpuResourceHandle{});
    static_assert(relationship.dependency == Renderer::GpuResourceHandle{});

    EXPECT_EQ(relationship.dependent, Renderer::GpuResourceHandle{});
    EXPECT_EQ(relationship.dependency, Renderer::GpuResourceHandle{});
}

TEST(GpuResourceDependencyTests, EqualRelationshipsCompareEqual) {
    constexpr Renderer::GpuResourceDependency original{
        .dependent = kDependentHandle,
        .dependency = kDependencyHandle,
    };

    constexpr Renderer::GpuResourceDependency equivalent{
        .dependent = kDependentHandle,
        .dependency = kDependencyHandle,
    };

    static_assert(original == equivalent);

    EXPECT_EQ(original, equivalent);
}

TEST(GpuResourceDependencyTests, RelationshipDirectionMatters) {
    constexpr Renderer::GpuResourceDependency forward{
        .dependent = kDependentHandle,
        .dependency = kDependencyHandle,
    };

    constexpr Renderer::GpuResourceDependency reverse{
        .dependent = kDependencyHandle,
        .dependency = kDependentHandle,
    };

    static_assert(!(forward == reverse));

    EXPECT_NE(forward, reverse);
}

TEST(GpuResourceDependencyTests, DetectsDifferentResourceIdentities) {
    const Renderer::GpuResourceDependency original{
        .dependent = kDependentHandle,
        .dependency = kDependencyHandle,
    };

    Renderer::GpuResourceDependency modified = original;

    ++modified.dependent.index;
    EXPECT_NE(original, modified);

    modified = original;
    ++modified.dependent.generation;
    EXPECT_NE(original, modified);

    modified = original;
    ++modified.dependent.registry_id;
    EXPECT_NE(original, modified);

    modified = original;
    ++modified.dependency.index;
    EXPECT_NE(original, modified);

    modified = original;
    ++modified.dependency.generation;
    EXPECT_NE(original, modified);

    modified = original;
    ++modified.dependency.registry_id;
    EXPECT_NE(original, modified);
}
