#include "engine/renderer/resource/gpu_resource_registry_id.hpp"
#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>
#include <set>
#include <thread>
#include <vector>

namespace Renderer = SNE::Engine::Renderer;

TEST(GpuResourceRegistryIdTests, AllocatesDistinctNonzeroIds) {
    const auto first = Renderer::allocateGpuResourceRegistryId();
    const auto second = Renderer::allocateGpuResourceRegistryId();
    const auto third = Renderer::allocateGpuResourceRegistryId();

    EXPECT_NE(first, 0U);
    EXPECT_NE(second, 0U);
    EXPECT_NE(third, 0U);

    EXPECT_NE(first, second);
    EXPECT_NE(first, third);
    EXPECT_NE(second, third);
}

TEST(GpuResourceRegistryIdTests, AllocatesUniqueIdsAcrossThreads) {
    constexpr std::size_t thread_count = 8U;
    constexpr std::size_t ids_per_thread = 64U;

    std::vector<std::uint64_t> ids(thread_count * ids_per_thread);
    std::vector<std::thread> threads;
    threads.reserve(thread_count);

    for (std::size_t thread_index = 0; thread_index < thread_count;
         ++thread_index) {
        threads.emplace_back([&ids, thread_index] -> void {
            for (std::size_t index = 0; index < ids_per_thread; ++index) {
                ids[(thread_index * ids_per_thread) + index] =
                    Renderer::allocateGpuResourceRegistryId();
            }
        });
    }

    for (auto &thread : threads) {
        thread.join();
    }

    const std::set<std::uint64_t> unique_ids(ids.begin(), ids.end());

    EXPECT_EQ(unique_ids.size(), ids.size());
    EXPECT_FALSE(unique_ids.contains(0U));
}
