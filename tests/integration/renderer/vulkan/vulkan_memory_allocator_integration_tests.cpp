#include "engine/renderer/gpu_memory_usage.hpp"
#include "engine/renderer/vulkan/vulkan_buffer.hpp"
#include "engine/renderer/vulkan/vulkan_memory_allocator.hpp"
#include "vulkan_integration_test_fixture.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <vulkan/vulkan.h>

namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = Renderer::Vulkan;

TEST_F(VulkanIntegrationTest, QueriesMemoryHeapBudgets) {
    VkPhysicalDeviceMemoryProperties memory_properties{};
    vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memory_properties);

    const std::vector<Vulkan::VulkanMemoryHeapBudget> memory_heap_budgets =
        memoryAllocator().queryMemoryHeapBudgets();

    ASSERT_EQ(memory_heap_budgets.size(), memory_properties.memoryHeapCount);

    for (const Vulkan::VulkanMemoryHeapBudget &budget : memory_heap_budgets) {
        EXPECT_NE(budget.budget, 0U);
    }
}

TEST_F(VulkanIntegrationTest, QueriesMemoryAllocationStatistics) {
    Vulkan::VulkanMemoryAllocator &memory_allocator = memoryAllocator();

    const Vulkan::VulkanMemoryAllocationStatistics memory_allocation_before =
        memory_allocator.queryMemoryAllocationStatistics();

    const Vulkan::VulkanBufferCreateInfo buffer_create_info{
        .size = 1U,
        .usage =
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Device,
    };

    Vulkan::VulkanBuffer device_buffer{memory_allocator, buffer_create_info};

    const Vulkan::VulkanMemoryAllocationStatistics memory_allocation_after =
        memory_allocator.queryMemoryAllocationStatistics();

    EXPECT_GT(memory_allocation_after.block_count, 0U);
    EXPECT_GT(memory_allocation_after.allocation_count,
              memory_allocation_before.allocation_count);
    EXPECT_GT(memory_allocation_after.allocation_bytes,
              memory_allocation_before.allocation_bytes);
}
