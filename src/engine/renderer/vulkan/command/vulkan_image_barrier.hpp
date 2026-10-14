#pragma once

#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Records an image memory barrier using Vulkan synchronization2.
     *
     * Records the supplied barrier in the command buffer without
     * submitting GPU work or tracking image layouts.
     *
     * The caller is responsible for supplying valid synchronization
     * scopes, image layouts, subresource ranges, and queue-family
     * ownership information.
     *
     * @param command_buffer Command buffer receiving the barrier.
     * @param barrier Image memory barrier to record.
     *
     * @pre command_buffer must be a valid recording command buffer.
     * @pre barrier.image must be a valid Vulkan image.
     * @pre barrier must satisfy Vulkan image memory barrier requirements.
     */
    auto recordImageBarrier(VkCommandBuffer command_buffer,
                            const VkImageMemoryBarrier2 &barrier) -> void;
} // namespace SNE::Engine::Renderer::Vulkan
