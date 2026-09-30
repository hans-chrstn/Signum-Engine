#pragma once

#include "engine/renderer/vulkan/vulkan_command_pool.hpp"
#include <cstdint>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Owns reusable Vulkan resources associated with one frame in
     * flight.
     *
     * Groups Vulkan resources whose lifetime and reuse are tied to a single
     * frame-in-flight slot.
     *
     * Each frame owns an independent command pool so command resources may be
     * reset and reused only after that frame's GPU work has completed.
     *
     * Additional per-frame command and synchronization resources are owned by
     * this type as the rendering frame lifecycle is established.
     *
     * The type is non-copyable because it owns Vulkan resources with exclusive
     * lifetimes. Ownership may be transferred through move operations.
     */
    class VulkanFrameResources {
      private:
        VulkanCommandPool m_CommandPool;

      public:
        /**
         * @brief Creates the Vulkan resources required by one frame-in-flight
         * slot.
         *
         * Creates a command pool associated with the graphics queue family.
         * Additional per-frame command and synchronization resources are
         * established here as the frame lifecycle is implemented.
         *
         * @param device Logical device used by the frame's Vulkan resources.
         * @param graphics_queue_family_index Queue-family index used for
         * graphics command recording and submission.
         *
         * @pre device must be a valid Vulkan logical-device handle.
         *
         * @throws Core::Error::EngineError if a required Vulkan resource cannot
         * be created.
         */
        VulkanFrameResources(VkDevice device,
                             std::uint32_t graphics_queue_family_index);

        /**
         * @brief Releases the resources owned by this frame-in-flight slot.
         */
        ~VulkanFrameResources() = default;

        VulkanFrameResources(const VulkanFrameResources &) = delete;

        auto operator=(const VulkanFrameResources &)
            -> VulkanFrameResources & = delete;

        /**
         * @brief Transfers ownership of one frame's Vulkan resources from
         * another object.
         *
         * @param other Frame-resource owner from which ownership is
         * transferred.
         */
        VulkanFrameResources(VulkanFrameResources &&other) noexcept = default;

        /**
         * @brief Replaces the currently owned frame resources by transferring
         * ownership from another object.
         *
         * @param other Frame-resource owner from which ownership is
         * transferred.
         *
         * @return Reference to this object.
         */
        auto operator=(VulkanFrameResources &&other) noexcept
            -> VulkanFrameResources & = default;
    };
} // namespace SNE::Engine::Renderer::Vulkan
