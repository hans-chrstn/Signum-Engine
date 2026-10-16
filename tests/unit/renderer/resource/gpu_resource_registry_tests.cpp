#include "engine/renderer/resource/gpu_resource_registry.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <utility>

namespace Renderer = SNE::Engine::Renderer;

namespace {
    struct CountingDeleter {
        int *destruction_count{};

        auto operator()(const int *value) const noexcept -> void {
            ++(*destruction_count);
            delete value;
        }
    };

    using TrackedResource = std::unique_ptr<int, CountingDeleter>;
} // namespace

TEST(GpuResourceRegistryTests, RejectsDefaultHandleOnEmptyRegistry) {
    Renderer::GpuResourceRegistry<std::unique_ptr<int>> registry;

    EXPECT_EQ(registry.find({}), nullptr);
    EXPECT_FALSE(registry.remove({}).has_value());
}

TEST(GpuResourceRegistryTests, InsertsMoveOnlyResource) {
    Renderer::GpuResourceRegistry<std::unique_ptr<int>> registry;

    const int num = 42;

    auto resource = std::make_unique<int>(num);

    const auto handle = registry.insert(std::move(resource));

    EXPECT_EQ(resource, nullptr);
    EXPECT_EQ(handle.index, 0U);
    EXPECT_EQ(handle.generation, 1U);

    auto *stored = registry.find(handle);

    ASSERT_NE(stored, nullptr);
    ASSERT_NE(*stored, nullptr);

    EXPECT_EQ(**stored, 42);
}

TEST(GpuResourceRegistryTests, AssignsSequentialSlotIndices) {
    Renderer::GpuResourceRegistry<std::unique_ptr<int>> registry;

    const auto first = registry.insert(std::make_unique<int>(10));
    const auto second = registry.insert(std::make_unique<int>(20));

    EXPECT_EQ(first.index, 0U);
    EXPECT_EQ(second.index, 1U);

    EXPECT_EQ(first.generation, 1U);
    EXPECT_EQ(second.generation, 1U);

    EXPECT_FALSE(first == second);
}

TEST(GpuResourceRegistryTests, FindsInsertedResource) {
    Renderer::GpuResourceRegistry<std::unique_ptr<int>> registry;

    const auto handle = registry.insert(std::make_unique<int>(42));

    auto *resource = registry.find(handle);

    ASSERT_NE(resource, nullptr);
    ASSERT_NE(*resource, nullptr);

    EXPECT_EQ(**resource, 42);
}

TEST(GpuResourceRegistryTests, RejectsInvalidResourceHandles) {
    Renderer::GpuResourceRegistry<std::unique_ptr<int>> registry;

    const auto valid = registry.insert(std::make_unique<int>(42));

    const Renderer::GpuResourceHandle invalid_generation{
        .index = valid.index,
        .generation = 0U,
    };

    const Renderer::GpuResourceHandle stale_generation{
        .index = valid.index,
        .generation = valid.generation + 1U,
    };

    const Renderer::GpuResourceHandle invalid_index{
        .index = valid.index + 1U,
        .generation = valid.generation,
    };

    EXPECT_EQ(registry.find({}), nullptr);
    EXPECT_EQ(registry.find(invalid_generation), nullptr);
    EXPECT_EQ(registry.find(stale_generation), nullptr);
    EXPECT_EQ(registry.find(invalid_index), nullptr);

    EXPECT_NE(registry.find(valid), nullptr);
}

TEST(GpuResourceRegistryTests, RemovesResourceAndTransfersOwnership) {
    Renderer::GpuResourceRegistry<std::unique_ptr<int>> registry;

    const auto handle = registry.insert(std::make_unique<int>(42));

    auto removed = registry.remove(handle);

    if (!removed.has_value()) {
        FAIL() << "Expected resource removal to return the owned resource";
        return;
    }

    ASSERT_NE(*removed, nullptr);

    EXPECT_EQ(**removed, 42);
    EXPECT_EQ(registry.find(handle), nullptr);
    EXPECT_FALSE(registry.remove(handle).has_value());
}

TEST(GpuResourceRegistryTests, InvalidRemovalPreservesExistingResource) {
    Renderer::GpuResourceRegistry<std::unique_ptr<int>> registry;

    const auto valid = registry.insert(std::make_unique<int>(42));

    const Renderer::GpuResourceHandle invalid_index{
        .index = valid.index + 1U,
        .generation = valid.generation,
    };

    const Renderer::GpuResourceHandle stale_generation{
        .index = valid.index,
        .generation = valid.generation + 1U,
    };

    EXPECT_FALSE(registry.remove({}).has_value());
    EXPECT_FALSE(registry.remove(invalid_index).has_value());
    EXPECT_FALSE(registry.remove(stale_generation).has_value());

    auto removed = registry.remove(valid);

    if (!removed.has_value()) {
        FAIL() << "Expected valid resource to remain removable";
        return;
    }

    ASSERT_NE(*removed, nullptr);

    EXPECT_EQ(**removed, 42);
}

TEST(GpuResourceRegistryTests, ReusesRemovedSlotWithNewGeneration) {
    Renderer::GpuResourceRegistry<std::unique_ptr<int>> registry;

    const auto original = registry.insert(std::make_unique<int>(42));

    auto removed = registry.remove(original);

    EXPECT_TRUE(removed.has_value());
    EXPECT_EQ(registry.find(original), nullptr);

    const auto replacement = registry.insert(std::make_unique<int>(100));

    EXPECT_EQ(replacement.index, original.index);
    EXPECT_EQ(replacement.generation, original.generation + 1U);

    EXPECT_EQ(registry.find(original), nullptr);

    auto *resource = registry.find(replacement);

    ASSERT_NE(resource, nullptr);
    ASSERT_NE(*resource, nullptr);

    EXPECT_EQ(**resource, 100);
}

TEST(GpuResourceRegistryTests, ReusesFreeSlotsInLastInFirstOutOrder) {
    Renderer::GpuResourceRegistry<std::unique_ptr<int>> registry;

    const auto first = registry.insert(std::make_unique<int>(10));
    const auto second = registry.insert(std::make_unique<int>(20));
    const auto third = registry.insert(std::make_unique<int>(30));

    auto removed_second = registry.remove(second);
    auto removed_third = registry.remove(third);

    EXPECT_TRUE(removed_second.has_value());
    EXPECT_TRUE(removed_third.has_value());

    const auto replacement_third = registry.insert(std::make_unique<int>(40));

    const auto replacement_second = registry.insert(std::make_unique<int>(50));

    EXPECT_EQ(replacement_third.index, third.index);
    EXPECT_EQ(replacement_second.index, second.index);

    EXPECT_EQ(replacement_third.generation, third.generation + 1U);
    EXPECT_EQ(replacement_second.generation, second.generation + 1U);

    EXPECT_EQ(registry.find(second), nullptr);
    EXPECT_EQ(registry.find(third), nullptr);

    EXPECT_NE(registry.find(first), nullptr);
    EXPECT_NE(registry.find(replacement_third), nullptr);
    EXPECT_NE(registry.find(replacement_second), nullptr);
}

TEST(GpuResourceRegistryTests, AppendsAfterFreeListIsExhausted) {
    Renderer::GpuResourceRegistry<std::unique_ptr<int>> registry;

    const auto first = registry.insert(std::make_unique<int>(10));
    const auto second = registry.insert(std::make_unique<int>(20));

    auto removed = registry.remove(first);

    EXPECT_TRUE(removed.has_value());

    const auto reused = registry.insert(std::make_unique<int>(30));
    const auto appended = registry.insert(std::make_unique<int>(40));

    EXPECT_EQ(reused.index, first.index);
    EXPECT_EQ(reused.generation, first.generation + 1U);

    EXPECT_EQ(appended.index, 2U);
    EXPECT_EQ(appended.generation, 1U);

    EXPECT_EQ(registry.find(first), nullptr);
    EXPECT_NE(registry.find(second), nullptr);
    EXPECT_NE(registry.find(reused), nullptr);
    EXPECT_NE(registry.find(appended), nullptr);
}

TEST(GpuResourceRegistryTests, KeepsEarlierHandlesStaleAcrossRepeatedReuse) {
    Renderer::GpuResourceRegistry<std::unique_ptr<int>> registry;

    const auto first = registry.insert(std::make_unique<int>(10));

    auto removed_first = registry.remove(first);

    EXPECT_TRUE(removed_first.has_value());

    const auto second = registry.insert(std::make_unique<int>(20));

    auto removed_second = registry.remove(second);

    EXPECT_TRUE(removed_second.has_value());

    const auto third = registry.insert(std::make_unique<int>(30));

    EXPECT_EQ(first.index, second.index);
    EXPECT_EQ(second.index, third.index);

    EXPECT_EQ(second.generation, first.generation + 1U);
    EXPECT_EQ(third.generation, first.generation + 2U);

    EXPECT_EQ(registry.find(first), nullptr);
    EXPECT_EQ(registry.find(second), nullptr);
    EXPECT_NE(registry.find(third), nullptr);

    EXPECT_FALSE(registry.remove(first).has_value());
    EXPECT_FALSE(registry.remove(second).has_value());
}

TEST(GpuResourceRegistryTests, RemovalPreservesResourceLifetime) {
    int destruction_count = 0;

    Renderer::GpuResourceRegistry<TrackedResource> registry;

    const auto handle = registry.insert(
        TrackedResource{new int{42}, CountingDeleter{&destruction_count}});

    EXPECT_EQ(destruction_count, 0);

    auto removed = registry.remove(handle);

    if (!removed.has_value()) {
        FAIL() << "Expected resource ownership to be transferred";
        return;
    }

    EXPECT_EQ(destruction_count, 0);
    EXPECT_NE(removed->get(), nullptr);
    EXPECT_EQ(registry.find(handle), nullptr);

    removed.reset();

    EXPECT_EQ(destruction_count, 1);
}
