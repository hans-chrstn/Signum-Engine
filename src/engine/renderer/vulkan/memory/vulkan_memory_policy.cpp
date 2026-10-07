#include "vulkan_memory_policy.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"

namespace SNE::Engine::Renderer::Vulkan {

    auto selectVulkanMemoryPolicy(GpuMemoryUsage memory_usage)
        -> VulkanMemoryPolicy {
        switch (memory_usage) {
        case GpuMemoryUsage::Device:
            return VulkanMemoryPolicy{
                .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                .flags = 0U,
            };
        case GpuMemoryUsage::Upload:
            return VulkanMemoryPolicy{
                .usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
                .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            };
        case GpuMemoryUsage::Readback:
            return VulkanMemoryPolicy{
                .usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
                .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
            };
        }
        Core::Assertion::failAssertion(
            Core::Assertion::AssertionType::Precondition,
            Core::Error::Subsystem::Vulkan, "Unsupported GPU memory usage");
    }
} // namespace SNE::Engine::Renderer::Vulkan
