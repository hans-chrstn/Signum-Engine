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
