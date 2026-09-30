#include "vulkan_frame_resources.hpp"

namespace SNE::Engine::Renderer::Vulkan {

    VulkanFrameResources::VulkanFrameResources(
        VkDevice device, std::uint32_t graphics_queue_family_index)
        : m_CommandPool(device, graphics_queue_family_index) {}

} // namespace SNE::Engine::Renderer::Vulkan
