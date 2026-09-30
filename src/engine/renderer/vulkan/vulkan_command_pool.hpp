#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    class VulkanCommandPool {
      private:
        VkDevice m_Device{VK_NULL_HANDLE};
        VkCommandPool m_CommandPool{VK_NULL_HANDLE};

      public:
        VulkanCommandPool(VkDevice device, std::uint32_t queue_family_index);
        ~VulkanCommandPool() noexcept;

        VulkanCommandPool(const VulkanCommandPool &) = delete;

        auto operator=(const VulkanCommandPool &)
            -> VulkanCommandPool & = delete;

        [[nodiscard]] auto nativeHandle() const noexcept -> VkCommandPool;
    };
} // namespace SNE::Engine::Renderer::Vulkan
