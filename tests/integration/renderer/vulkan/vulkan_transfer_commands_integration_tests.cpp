#include "engine/renderer/gpu_memory_usage.hpp"
#include "engine/renderer/vulkan/vulkan_buffer.hpp"
#include "engine/renderer/vulkan/vulkan_immediate_submission.hpp"
#include "engine/renderer/vulkan/vulkan_transfer_commands.hpp"
#include "vulkan_integration_test_fixture.hpp"
#include <cstddef>
#include <gtest/gtest.h>
#include <vulkan/vulkan.h>

namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = Renderer::Vulkan;

TEST_F(VulkanIntegrationTest,
       RecordBufferCopyRejectsSourceWithoutTransferSourceUsage) {
    Vulkan::VulkanMemoryAllocator &memory_allocator = memoryAllocator();

    const Vulkan::VulkanBufferCreateInfo source_create_info{
        .size = 16U,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Device,
    };

    const Vulkan::VulkanBufferCreateInfo destination_create_info{
        .size = 16U,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Device,
    };

    const Vulkan::VulkanBuffer source{memory_allocator, source_create_info};
    const Vulkan::VulkanBuffer destination{memory_allocator,
                                           destination_create_info};

    std::byte dummy{};
    auto *command_buffer = reinterpret_cast<VkCommandBuffer>(&dummy);

    const Vulkan::BufferCopyRequest copy_request{
        .source = source,
        .destination = destination,
        .source_offset = 0U,
        .destination_offset = 0U,
        .size = 4U,
    };

    ASSERT_DEATH(Vulkan::recordBufferCopy(command_buffer, copy_request),
                 "recordBufferCopy requires the source buffer to support "
                 "transfer-source usage");
}

TEST_F(VulkanIntegrationTest,
       RecordBufferCopyRejectsDestinationWithoutTransferDestinationUsage) {
    Vulkan::VulkanMemoryAllocator &memory_allocator = memoryAllocator();

    const Vulkan::VulkanBufferCreateInfo source_create_info{
        .size = 16U,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Device,
    };

    const Vulkan::VulkanBufferCreateInfo destination_create_info{
        .size = 16U,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Device,
    };

    const Vulkan::VulkanBuffer source{memory_allocator, source_create_info};
    const Vulkan::VulkanBuffer destination{memory_allocator,
                                           destination_create_info};

    std::byte dummy{};
    auto *command_buffer = reinterpret_cast<VkCommandBuffer>(&dummy);

    const Vulkan::BufferCopyRequest copy_request{
        .source = source,
        .destination = destination,
        .source_offset = 0U,
        .destination_offset = 0U,
        .size = 4U,
    };

    ASSERT_DEATH(Vulkan::recordBufferCopy(command_buffer, copy_request),
                 "recordBufferCopy requires the destination buffer to support "
                 "transfer-destination usage");
}

TEST_F(VulkanIntegrationTest,
       RecordBufferCopyRejectsOverlappingSameBufferRanges) {
    Vulkan::VulkanMemoryAllocator &memory_allocator = memoryAllocator();

    const Vulkan::VulkanBufferCreateInfo buffer_create_info{
        .size = 16U,
        .usage =
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Device,
    };

    const Vulkan::VulkanBuffer buffer{memory_allocator, buffer_create_info};

    std::byte dummy{};
    auto *command_buffer = reinterpret_cast<VkCommandBuffer>(&dummy);

    const Vulkan::BufferCopyRequest copy_request{
        .source = buffer,
        .destination = buffer,
        .source_offset = 0U,
        .destination_offset = 2U,
        .size = 4U,
    };

    ASSERT_DEATH(
        Vulkan::recordBufferCopy(command_buffer, copy_request),
        "recordBufferCopy requires non-overlapping ranges when copying "
        "within the same buffer");
}

TEST_F(VulkanIntegrationTest,
       RecordBufferCopyAllowsNonOverlappingSameBufferRanges) {
    Vulkan::VulkanMemoryAllocator &memory_allocator = memoryAllocator();

    VkQueue graphics_queue{VK_NULL_HANDLE};

    vkGetDeviceQueue(m_Device, m_SelectedQueueFamily.family_index, 0U,
                     &graphics_queue);

    ASSERT_NE(graphics_queue, VK_NULL_HANDLE);

    Vulkan::VulkanImmediateSubmission immediate_submission{
        m_Device,
        graphics_queue,
        m_SelectedQueueFamily.family_index,
    };

    const Vulkan::VulkanBufferCreateInfo buffer_create_info{
        .size = 8U,
        .usage =
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Device,
    };

    const Vulkan::VulkanBuffer buffer{memory_allocator, buffer_create_info};

    const Vulkan::BufferCopyRequest copy_request{
        .source = buffer,
        .destination = buffer,
        .source_offset = 0U,
        .destination_offset = 4U,
        .size = 4U,
    };

    EXPECT_NO_THROW(immediate_submission.execute(
        [&](VkCommandBuffer command_buffer) -> void {
            Vulkan::recordBufferCopy(command_buffer, copy_request);
        }));
}
