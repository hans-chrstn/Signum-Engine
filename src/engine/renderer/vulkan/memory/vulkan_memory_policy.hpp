#pragma once

#include "engine/renderer/gpu_memory_usage.hpp"
#include <vk_mem_alloc.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Describes VMA allocation policy derived from engine-level GPU
     * memory intent.
     *
     * Stores the VMA memory preference and allocation flags required to
     * implement a selected GPU memory usage policy.
     *
     * This structure does not own any Vulkan or VMA resources.
     */
    struct VulkanMemoryPolicy {
        /** VMA memory placement preference for the allocation. */
        VmaMemoryUsage usage{VMA_MEMORY_USAGE_AUTO};
        /** Additional VMA allocation behavior flags. */
        VmaAllocationCreateFlags flags{0};
    };

    /**
     * @brief Selects the Vulkan/VMA allocation policy for a GPU memory usage.
     *
     * Translates engine-level GPU memory intent into the VMA memory preference
     * and allocation flags used by the Vulkan backend.
     *
     * @param memory_usage Engine-level GPU memory usage to translate.
     *
     * @return Vulkan/VMA allocation policy implementing the requested usage.
     */
    [[nodiscard]] auto selectVulkanMemoryPolicy(GpuMemoryUsage memory_usage)
        -> VulkanMemoryPolicy;
} // namespace SNE::Engine::Renderer::Vulkan
