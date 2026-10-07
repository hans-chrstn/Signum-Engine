#include "engine/renderer/gpu_memory_usage.hpp"
#include "engine/renderer/vulkan/vulkan_buffer.hpp"
#include "vulkan_integration_test_fixture.hpp"
#include <array>
#include <cstddef>
#include <gtest/gtest.h>
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
