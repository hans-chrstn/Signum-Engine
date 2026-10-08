#pragma once

#include "engine/renderer/vulkan/memory/vulkan_buffer.hpp"
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Describes a buffer-to-buffer copy operation.
     *
     * Identifies the source and destination buffers together with the byte
     * ranges used when recording a Vulkan buffer copy command.
     *
     * The referenced buffers are non-owning and must remain valid until the
     * recorded GPU operation has finished executing.
     *
     * This structure describes transfer commands only. It does not describe
     * queue submission, synchronization, or queue-family ownership.
     */
    struct BufferCopyRequest {
        /**
         * @brief Source buffer to copy data from.
         */
        const VulkanBuffer &source;

        /**
         * @brief Destination buffer to copy data into.
         */
        const VulkanBuffer &destination;

        /**
         * @brief Byte offset within the source buffer where the copy begins.
         */
        VkDeviceSize source_offset{};

        /**
         * @brief Byte offset within the destination buffer where the copy
         * begins.
         */
        VkDeviceSize destination_offset{};

        /**
         * @brief Number of bytes to copy.
         */
        VkDeviceSize size{};
    };

    /**
     * @brief Records a buffer-to-buffer copy command.
     *
     * Records a copy from the requested source buffer range into the requested
     * destination buffer range using the supplied command buffer.
     *
     * This function records transfer commands only. It does not submit the
     * command buffer, select a Vulkan queue, synchronize queue execution, or
     * perform queue-family ownership transfers.
     *
     * The command buffer must ultimately be submitted to a queue family that is
     * permitted to access both buffers. Signum currently creates Vulkan buffers
     * using VK_SHARING_MODE_EXCLUSIVE, so transferring a resource between
     * different queue families requires explicit Vulkan queue-family ownership
     * release and acquire operations.
     *
     * Signum's current transfer path submits transfer work through the graphics
     * queue. A dedicated transfer queue belonging to a different queue family
     * must not be introduced until queue-family ownership transfer and
     * cross-queue synchronization are implemented.
     *
     * Keeping transfer-command recording independent from queue submission
     * allows the submission policy to evolve later without coupling buffer-copy
     * command construction to a specific queue.
     *
     * @param command_buffer Command buffer receiving the transfer command.
     * @param buffer_copy_request Description of the buffers and byte range to
     * copy.
     *
     * @pre command_buffer must be a valid Vulkan command buffer.
     * @pre buffer_copy_request.size must be greater than zero.
     * @pre The source buffer must have been created with
     * VK_BUFFER_USAGE_TRANSFER_SRC_BIT.
     * @pre The destination buffer must have been created with
     * VK_BUFFER_USAGE_TRANSFER_DST_BIT.
     * @pre The requested source range must fit within the source buffer.
     * @pre The requested destination range must fit within the destination
     * buffer.
     * @pre When the source and destination refer to the same Vulkan buffer, the
     * requested source and destination ranges must not overlap.
     * @pre The queue family used to submit the command buffer must be permitted
     * to access both buffers, or the caller must establish the required
     * queue-family ownership transfers before the resources are used.
     */
    auto recordBufferCopy(VkCommandBuffer command_buffer,
                          const BufferCopyRequest &buffer_copy_request) -> void;
} // namespace SNE::Engine::Renderer::Vulkan
