#include "engine/renderer/gpu_memory_usage.hpp"
#include "engine/renderer/vulkan/vulkan_memory_policy.hpp"
#include <gtest/gtest.h>

namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = Renderer::Vulkan;

TEST(VulkanMemoryPolicyTests, SelectsDeviceMemoryPolicy) {
    const Renderer::GpuMemoryUsage mem_usage = Renderer::GpuMemoryUsage::Device;
    const Vulkan::VulkanMemoryPolicy expected_policy{
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        .flags = 0U,
    };

    const Vulkan::VulkanMemoryPolicy selected_policy =
        Vulkan::selectVulkanMemoryPolicy(mem_usage);

    EXPECT_EQ(selected_policy.usage, expected_policy.usage);
    EXPECT_EQ(selected_policy.flags, expected_policy.flags);
}

TEST(VulkanMemoryPolicyTests, SelectsUploadMemoryPolicy) {
    const Renderer::GpuMemoryUsage mem_usage = Renderer::GpuMemoryUsage::Upload;
    const Vulkan::VulkanMemoryPolicy expected_policy{
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
    };

    const Vulkan::VulkanMemoryPolicy selected_policy =
        Vulkan::selectVulkanMemoryPolicy(mem_usage);

    EXPECT_EQ(selected_policy.usage, expected_policy.usage);
    EXPECT_EQ(selected_policy.flags, expected_policy.flags);
}

TEST(VulkanMemoryPolicyTests, SelectsReadbackMemoryPolicy) {
    const Renderer::GpuMemoryUsage mem_usage =
        Renderer::GpuMemoryUsage::Readback;
    const Vulkan::VulkanMemoryPolicy expected_policy{
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
    };

    const Vulkan::VulkanMemoryPolicy selected_policy =
        Vulkan::selectVulkanMemoryPolicy(mem_usage);

    EXPECT_EQ(selected_policy.usage, expected_policy.usage);
    EXPECT_EQ(selected_policy.flags, expected_policy.flags);
}
