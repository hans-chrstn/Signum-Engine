#pragma once

#include "engine/renderer/vulkan/vulkan_command_pool.hpp"
#include "engine/renderer/vulkan/vulkan_fence.hpp"
#include "engine/renderer/vulkan/vulkan_semaphore.hpp"
#include <cstdint>
#include <span>
#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Owns reusable Vulkan resources associated with one frame in
     * flight.
     *
     * Groups Vulkan resources whose lifetime and reuse are tied to a single
     * frame-in-flight slot.
     *
     * Each frame owns an independent command pool and command-buffer storage so
     * command resources may be reused only after that frame's GPU work has
     * completed.
     *
     * The frame also owns an image-availability semaphore used to synchronize
     * swapchain-image acquisition with GPU submission and an in-flight fence
     * used to determine when the frame slot may be safely reused.
     *
     * The type is non-copyable because it owns Vulkan resources with exclusive
     * lifetimes. Ownership may be transferred through move operations.
     */
    class VulkanFrameResources {
      private:
        /**
         * @brief Command pool owned by this frame-in-flight slot.
         *
         * Allocates command buffers associated with the graphics queue family
         * and remains alive for the lifetime of those command buffers.
         */
        VulkanCommandPool m_CommandPool;
        /**
         * @brief Command-buffer handles allocated from this frame's command
         * pool.
         *
         * The handles are non-owning with respect to Vulkan allocation
         * lifetime; their validity is tied to m_CommandPool.
         *
         * The container owns only the stored handle values.
         */
        std::vector<VkCommandBuffer> m_CommandBuffers;
        /**
         * @brief Semaphore signaled when a swapchain image becomes available.
         *
         * Used to synchronize swapchain-image acquisition with GPU work
         * submitted for this frame-in-flight slot.
         */
        VulkanSemaphore m_ImageAvailableSemaphore;
        /**
         * @brief Fence used to determine when this frame-in-flight slot may be
         * reused.
         *
         * The fence begins in the signaled state so the frame slot is
         * immediately reusable before its first GPU submission.
         *
         * Subsequent submissions signal the fence when the GPU work associated
         * with this frame slot has completed.
         */
        VulkanFence m_InFlightFence;

      public:
        /**
         * @brief Creates the Vulkan resources required by one frame-in-flight
         * slot.
         *
         * Creates a command pool associated with the graphics queue family,
         * allocates the initial command-buffer set, creates the
         * image-availability semaphore, and creates the in-flight fence in the
         * signaled state.
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

        /**
         * @brief Returns the command buffers associated with this frame.
         *
         * Provides a non-owning read-only view of the command-buffer handles
         * stored by this frame-in-flight resource set.
         *
         * The returned span remains valid only while this object exists and its
         * command-buffer storage is not modified.
         *
         * The command buffers remain associated with the command pool owned by
         * this frame and must not outlive that pool.
         *
         * @return Read-only non-owning view of this frame's command-buffer
         * handles.
         */
        [[nodiscard]] auto commandBuffers() const noexcept
            -> std::span<const VkCommandBuffer>;
    };
} // namespace SNE::Engine::Renderer::Vulkan
