#include "vulkan_frame_resources.hpp"

namespace SNE::Engine::Renderer::Vulkan {

    VulkanFrameResources::VulkanFrameResources(
        VkDevice device, std::uint32_t graphics_queue_family_index)
        : m_CommandPool(device, graphics_queue_family_index),
          m_CommandBuffers(m_CommandPool.allocateCommandBuffers(
              VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1U)) {}

    auto VulkanFrameResources::commandBuffers() const noexcept
        -> std::span<const VkCommandBuffer> {
        return m_CommandBuffers;
    }

} // namespace SNE::Engine::Renderer::Vulkan
