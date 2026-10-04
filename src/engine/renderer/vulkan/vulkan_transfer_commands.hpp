#pragma once

#include "vulkan_buffer.hpp"
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
     */
    struct BufferCopyRequest {
        const VulkanBuffer &source;
        const VulkanBuffer &destination;
        VkDeviceSize source_offset{};
        VkDeviceSize destination_offset{};
        VkDeviceSize size{};
    };

    /**
     * @brief Records a buffer-to-buffer copy command.
     *
     * Records a copy from the requested source buffer range into the requested
     * destination buffer range using the supplied command buffer.
     *
     * This function only records the transfer. The copy does not execute until
     * the command buffer is submitted to a compatible Vulkan queue.
     *
     * @param command_buffer Command buffer receiving the transfer command.
     * @param buffer_copy_request Description of the buffers and byte range to
     * copy.
     *
     * @pre command_buffer must be a valid Vulkan command buffer.
     * @pre buffer_copy_request.size must be greater than zero.
     * @pre The requested source range must fit within the source buffer.
     * @pre The requested destination range must fit within the destination
     * buffer.
     */
    auto recordBufferCopy(VkCommandBuffer command_buffer,
                          const BufferCopyRequest &buffer_copy_request) -> void;
} // namespace SNE::Engine::Renderer::Vulkan
