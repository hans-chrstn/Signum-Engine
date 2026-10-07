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
     * The command pool is created for the queue family supplied during
     * construction, so commands recorded through this object are intended for
     * submission to the associated queue.
     *
     * An instance permits only one active execution at a time. Concurrent or
     * recursive calls to execute() on the same instance are contract
     * violations.
     *
     * Queue access is not synchronized against other users of the borrowed
     * queue. The caller must ensure that host access to the queue is externally
     * synchronized while execute() performs queue operations.
     *
     * The current renderer constructs its immediate-submission path using the
     * graphics queue and graphics queue family. This is also the current
     * baseline for synchronous transfer work.
     *
     * Signum currently creates normal Vulkan buffers and images using
     * VK_SHARING_MODE_EXCLUSIVE. Therefore, a dedicated transfer queue
     * belonging to a different queue family must not replace the graphics-queue
     * transfer path until explicit queue-family ownership transfers and the
     * required cross-queue synchronization are implemented.
     *
     * This type remains generic with respect to the commands recorded by
     * execute(). It is not an upload-specific abstraction and should remain
     * reusable for synchronous one-time Vulkan work.
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
         * @brief Creates resources for synchronous immediate submission to a
         * Vulkan queue.
         *
         * Creates a command pool for the supplied queue family, allocates the
         * reusable command buffer from that pool, and creates the fence used to
         * wait for submitted work to complete.
         *
         * The supplied queue is borrowed and must remain valid for the lifetime
         * of this object. The queue must belong to queue_family_index.
         *
         * The current renderer uses the graphics queue and graphics queue
         * family for this object. If a future submission path uses a queue from
         * another family, resources accessed by that queue must satisfy Vulkan
         * queue-family ownership requirements.
         *
         * @param device Logical device used to create submission resources.
         * @param queue Queue used to submit recorded work.
         * @param queue_family_index Queue-family index to which the supplied
         * queue belongs.
         *
         * @pre device must be a valid Vulkan logical device.
         * @pre queue must be a valid Vulkan queue.
         * @pre queue_family_index must identify the queue family that owns
         * queue.
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
         * This function controls command recording and synchronous queue
         * submission, but it does not automatically establish queue-family
         * ownership transfers for resources referenced by recorded commands.
         *
         * Resources used by the recorded commands must therefore already be
         * accessible from the queue family associated with this
         * immediate-submission object, or the recorded commands and surrounding
         * synchronization must establish the required Vulkan queue-family
         * ownership transitions.
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
         * @pre Resources referenced by the recorded commands must be accessible
         * from the associated queue family or have the required queue-family
         * ownership transitions established explicitly.
         *
         * @throws Core::Error::EngineError if Vulkan fails while resetting,
         * beginning, ending, submitting, or waiting for the immediate work.
         */
        auto execute(const std::function<void(VkCommandBuffer)> &record)
            -> void;
    };
} // namespace SNE::Engine::Renderer::Vulkan
