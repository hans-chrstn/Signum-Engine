#include "../vulkan_integration_test_fixture.hpp"
#include "engine/renderer/gpu_memory_usage.hpp"
#include "engine/renderer/vulkan/memory/vulkan_buffer.hpp"
#include <array>
#include <cstddef>
#include <gtest/gtest.h>
#include <utility>
#include <vulkan/vulkan.h>

namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = Renderer::Vulkan;

TEST_F(VulkanIntegrationTest, CreatesAndDestroysVmaBackedBuffer) {
    const Vulkan::VulkanBufferCreateInfo buffer_create_info{
        .size = 256U,
        .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Device,
    };

    Vulkan::VulkanBuffer buffer{memoryAllocator(), buffer_create_info};
}

TEST_F(VulkanIntegrationTest, WritesToVmaBackedUploadBuffer) {
    const std::array<std::byte, 4> bytes{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
    };

    Vulkan::VulkanBufferCreateInfo buffer_create_info{};
    buffer_create_info.size = static_cast<VkDeviceSize>(bytes.size());
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_create_info.memory_usage = Renderer::GpuMemoryUsage::Upload;

    Vulkan::VulkanBuffer buffer{memoryAllocator(), buffer_create_info};

    buffer.write(bytes);
}

TEST_F(VulkanIntegrationTest, MoveConstructsVmaBackedBuffer) {
    const std::array<std::byte, 4> source_bytes{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
    };

    const Vulkan::VulkanBufferCreateInfo source_create_info{
        .size = static_cast<VkDeviceSize>(source_bytes.size()),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Device,
    };

    const Vulkan::VulkanMemoryAllocationStatistics statistics_before =
        memoryAllocator().queryMemoryAllocationStatistics();

    {
        Vulkan::VulkanBuffer source{memoryAllocator(), source_create_info};

        const VkBuffer original_handle = source.nativeHandle();
        const VkDeviceSize original_size = source.size();
        const VkBufferUsageFlags original_usage = source.usage();

        Vulkan::VulkanBuffer destination{std::move(source)};

        EXPECT_EQ(destination.nativeHandle(), original_handle);
        EXPECT_EQ(destination.size(), original_size);
        EXPECT_EQ(destination.usage(), original_usage);

        const Vulkan::VulkanMemoryAllocationStatistics statistics_after_move =
            memoryAllocator().queryMemoryAllocationStatistics();

        EXPECT_EQ(statistics_after_move.allocation_count,
                  statistics_before.allocation_count + 1U);
    }

    const Vulkan::VulkanMemoryAllocationStatistics
        statistics_after_destruction =
            memoryAllocator().queryMemoryAllocationStatistics();

    EXPECT_EQ(statistics_after_destruction.allocation_count,
              statistics_before.allocation_count);
}

TEST_F(VulkanIntegrationTest, MoveAssignsVmaBackedBuffer) {
    const std::array<std::byte, 4> source_bytes{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
    };

    const std::array<std::byte, 8> destination_bytes{
        std::byte{0x01}, std::byte{0x02}, std::byte{0x03}, std::byte{0x04},
        std::byte{0x05}, std::byte{0x06}, std::byte{0x07}, std::byte{0x08},
    };

    const Vulkan::VulkanBufferCreateInfo source_create_info{
        .size = static_cast<VkDeviceSize>(source_bytes.size()),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Device,
    };

    const Vulkan::VulkanBufferCreateInfo destination_create_info{
        .size = static_cast<VkDeviceSize>(destination_bytes.size()),
        .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Device,
    };

    const Vulkan::VulkanMemoryAllocationStatistics statistics_before =
        memoryAllocator().queryMemoryAllocationStatistics();

    {
        Vulkan::VulkanBuffer source{memoryAllocator(), source_create_info};
        Vulkan::VulkanBuffer destination{memoryAllocator(),
                                         destination_create_info};

        const VkBuffer original_source_handle = source.nativeHandle();
        const VkBuffer original_destination_handle = destination.nativeHandle();

        const VkDeviceSize original_source_size = source.size();
        const VkBufferUsageFlags original_source_usage = source.usage();

        EXPECT_NE(original_source_handle, VK_NULL_HANDLE);
        EXPECT_NE(original_destination_handle, VK_NULL_HANDLE);
        EXPECT_NE(original_source_handle, original_destination_handle);

        const Vulkan::VulkanMemoryAllocationStatistics statistics_before_move =
            memoryAllocator().queryMemoryAllocationStatistics();

        EXPECT_EQ(statistics_before_move.allocation_count,
                  statistics_before.allocation_count + 2U);

        destination = std::move(source);

        EXPECT_EQ(destination.nativeHandle(), original_source_handle);
        EXPECT_NE(destination.nativeHandle(), original_destination_handle);
        EXPECT_EQ(destination.size(), original_source_size);
        EXPECT_EQ(destination.usage(), original_source_usage);

        const Vulkan::VulkanMemoryAllocationStatistics statistics_after_move =
            memoryAllocator().queryMemoryAllocationStatistics();

        EXPECT_EQ(statistics_after_move.allocation_count,
                  statistics_before.allocation_count + 1U);
    }

    const Vulkan::VulkanMemoryAllocationStatistics
        statistics_after_destruction =
            memoryAllocator().queryMemoryAllocationStatistics();

    EXPECT_EQ(statistics_after_destruction.allocation_count,
              statistics_before.allocation_count);
}
