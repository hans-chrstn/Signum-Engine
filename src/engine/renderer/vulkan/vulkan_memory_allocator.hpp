#pragma once

#include <cstdint>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Owns the Vulkan Memory Allocator used by the Vulkan renderer.
     *
     * Creates and manages the lifetime of a VMA allocator associated with a
     * Vulkan instance, physical device, and logical device.
     *
     * The supplied Vulkan handles are borrowed and must remain valid for the
     * lifetime of this object.
     *
     * GPU resources allocated through this allocator must be released before
     * the allocator is destroyed.
     *
     * The type is non-copyable because it exclusively owns the VMA allocator.
     */
    class VulkanMemoryAllocator {
      private:
        VmaAllocator m_Allocator{nullptr};

      public:
        /**
         * @brief Creates the Vulkan memory allocator.
         *
         * @param instance Vulkan instance used by the allocator.
         * @param physical_device Physical device whose memory capabilities are
         * used.
         * @param device Logical device used for Vulkan memory operations.
         * @param api_version Vulkan API version used by the renderer.
         *
         * @throws Core::Error::EngineError if the VMA allocator cannot be
         * created.
         */
        VulkanMemoryAllocator(VkInstance instance,
                              VkPhysicalDevice physical_device, VkDevice device,
                              std::uint32_t api_version);

        /**
         * @brief Releases the owned VMA allocator.
         */
        ~VulkanMemoryAllocator() noexcept;
        VulkanMemoryAllocator(const VulkanMemoryAllocator &) = delete;

        auto operator=(const VulkanMemoryAllocator &)
            -> VulkanMemoryAllocator & = delete;

        /**
         * @brief Returns the underlying VMA allocator handle.
         *
         * The returned handle is non-owning and remains valid only while this
         * VulkanMemoryAllocator object remains alive.
         *
         * @return VMA allocator owned by this object.
         */
        [[nodiscard]] auto nativeHandle() const noexcept -> VmaAllocator;
    };
} // namespace SNE::Engine::Renderer::Vulkan
