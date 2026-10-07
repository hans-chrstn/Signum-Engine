#pragma once

#include "engine/renderer/vulkan/synchronization/vulkan_fence.hpp"
#include "vulkan_command_pool.hpp"
#include <atomic>
#include <cstdint>
#include <functional>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Owns resources for synchronous one-time Vulkan submissions.
     *
     * Maintains a command pool, command buffer, and fence associated with a
     * supplied Vulkan queue. Recorded work may be submitted and waited on
     * synchronously before the submission resources are reused.
     *
     * The Vulkan queue is borrowed and must remain valid for the lifetime of
     * this object. The command pool and fence are owned by this object.
     *
     * An instance permits only one active execution at a time. Concurrent or
     * recursive calls to execute() on the same instance are contract
     * violations.
     *
     * Queue access is not synchronized against other users of the borrowed
     * queue. The caller must ensure that host access to the queue is externally
     * synchronized while execute() performs queue operations.
     */
    class VulkanImmediateSubmission {
      private:
        /** Non-owning queue handle used for immediate submissions. */
        VkQueue m_Queue{VK_NULL_HANDLE};
        /** Command pool owned for allocating the immediate command buffer. */
        VulkanCommandPool m_VulkanCommandPool;
        /**
         * Non-owning command-buffer handle allocated from m_VulkanCommandPool.
         */
        VkCommandBuffer m_CommandBuffer{VK_NULL_HANDLE};
        /** Fence owned for waiting until immediate submissions complete. */
        VulkanFence m_VulkanFence;
        /**
         * @brief Tracks whether an immediate submission is currently executing.
         *
         * Used to reject overlapping or recursive reuse of the
         * immediate-submission resources.
         */
        std::atomic_bool m_Executing{false};

      public:
        /**
         * @brief Creates resources for immediate submission to a Vulkan queue.
         *
         * @param device Logical device used to create submission resources.
         * @param queue Queue used to submit recorded work.
         * @param queue_family_index Queue-family index to which the supplied
         * queue belongs.
         *
         * @pre device must be a valid Vulkan logical device.
         * @pre queue must be a valid Vulkan queue.
         */
        VulkanImmediateSubmission(VkDevice device, VkQueue queue,
                                  std::uint32_t queue_family_index);

        /**
         * @brief Releases the owned immediate-submission resources.
         */
        ~VulkanImmediateSubmission() noexcept = default;

        VulkanImmediateSubmission(const VulkanImmediateSubmission &) = delete;
        auto operator=(const VulkanImmediateSubmission &)
            -> VulkanImmediateSubmission & = delete;

        /**
         * @brief Records and synchronously executes one-time Vulkan commands.
         *
         * Resets the reusable immediate-submission resources, begins
         * command-buffer recording, invokes the supplied recording callback
         * with the owned command buffer, ends recording, submits the command
         * buffer to the associated queue, and waits until execution completes.
         *
         * The function does not return until the submitted GPU work has
         * finished.
         *
         * @param record Callback invoked while the command buffer is recording.
         * The callback receives the non-owning Vulkan command-buffer handle
         * into which commands should be recorded.
         *
         * @pre record must contain a valid callable.
         * @pre No other execution may currently be active on this
         * immediate-submission object, including recursive execution from the
         * recording callback.
         * @pre Host access to the borrowed Vulkan queue must be externally
         * synchronized for the duration of this operation.
         *
         * @throws Core::Error::EngineError if Vulkan fails while resetting,
         * beginning, ending, submitting, or waiting for the immediate work.
         */
        auto execute(const std::function<void(VkCommandBuffer)> &record)
            -> void;
    };
} // namespace SNE::Engine::Renderer::Vulkan
