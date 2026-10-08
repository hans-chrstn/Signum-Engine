#include "../vulkan_integration_test_fixture.hpp"
#include "engine/renderer/gpu_memory_usage.hpp"
#include "engine/renderer/vulkan/memory/vulkan_buffer.hpp"
#include "engine/renderer/vulkan/memory/vulkan_readback.hpp"
#include "engine/renderer/vulkan/memory/vulkan_upload.hpp"
#include <array>
#include <cstddef>
#include <gtest/gtest.h>
#include <vulkan/vulkan.h>

namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = Renderer::Vulkan;

TEST_F(VulkanIntegrationTest, UploadsAndReadsBackBufferData) {
    Vulkan::VulkanMemoryAllocator &memory_allocator = memoryAllocator();

    const std::array<std::byte, 4> input_bytes{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
    };

    std::array<std::byte, 4> output_bytes{
        std::byte{0x00},
        std::byte{0x00},
        std::byte{0x00},
        std::byte{0x00},
    };

    const auto input_size = static_cast<VkDeviceSize>(input_bytes.size());

    const Vulkan::VulkanBufferCreateInfo buffer_create_info{
        .size = input_size,
        .usage =
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Device,
    };

    Vulkan::VulkanBuffer device_buffer{memory_allocator, buffer_create_info};

    Vulkan::uploadBufferData(memory_allocator, immediateSubmission(),
                             device_buffer, input_bytes);

    Vulkan::readBufferData(memory_allocator, immediateSubmission(),
                           device_buffer, output_bytes);

    EXPECT_EQ(input_bytes, output_bytes);
}

TEST_F(VulkanIntegrationTest, UploadsPartialDataAtDestinationOffset) {
    constexpr std::size_t buffer_size = 8U;
    constexpr VkDeviceSize destination_offset = 5U;

    Vulkan::VulkanMemoryAllocator &memory_allocator = memoryAllocator();

    const std::array<std::byte, buffer_size> initial_bytes{
        std::byte{0x01}, std::byte{0x02}, std::byte{0x03}, std::byte{0x04},
        std::byte{0x05}, std::byte{0x06}, std::byte{0x07}, std::byte{0x08},
    };

    const std::array<std::byte, 3> replacement_bytes{
        std::byte{0xAA},
        std::byte{0xBB},
        std::byte{0xCC},
    };

    const std::array<std::byte, buffer_size> expected_bytes{
        std::byte{0x01}, std::byte{0x02}, std::byte{0x03}, std::byte{0x04},
        std::byte{0x05}, std::byte{0xAA}, std::byte{0xBB}, std::byte{0xCC},
    };

    std::array<std::byte, buffer_size> output_bytes{};

    const Vulkan::VulkanBufferCreateInfo buffer_create_info{
        .size = static_cast<VkDeviceSize>(initial_bytes.size()),
        .usage =
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Device,
    };

    Vulkan::VulkanBuffer device_buffer{memory_allocator, buffer_create_info};

    Vulkan::uploadBufferData(memory_allocator, immediateSubmission(),
                             device_buffer, initial_bytes);

    Vulkan::uploadBufferData(memory_allocator, immediateSubmission(),
                             device_buffer, replacement_bytes,
                             destination_offset);

    Vulkan::readBufferData(memory_allocator, immediateSubmission(),
                           device_buffer, output_bytes);

    EXPECT_EQ(output_bytes, expected_bytes);
}

TEST_F(VulkanIntegrationTest, RejectsUploadRangeBeyondDestinationBuffer) {
    constexpr VkDeviceSize buffer_size = 8U;
    constexpr VkDeviceSize invalid_destination_offset = 6U;

    Vulkan::VulkanMemoryAllocator &memory_allocator = memoryAllocator();

    const std::array<std::byte, 3> input_bytes{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
    };

    const Vulkan::VulkanBufferCreateInfo buffer_create_info{
        .size = buffer_size,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Device,
    };

    const Vulkan::VulkanBuffer device_buffer{memory_allocator,
                                             buffer_create_info};

    ASSERT_DEATH(Vulkan::uploadBufferData(
                     memory_allocator, immediateSubmission(), device_buffer,
                     input_bytes, invalid_destination_offset),
                 "uploadBufferData range exceeds the destination buffer size");
}
