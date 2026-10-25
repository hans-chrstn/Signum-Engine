#pragma once

#include "engine/renderer/vulkan/memory/vulkan_buffer.hpp"
#include "engine/renderer/vulkan/memory/vulkan_image.hpp"
#include <span>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Describes a buffer-to-image copy operation.
     *
     * Identifies the source buffer, destination image, destination
     * layout, and image subresources receiving copied data.
     *
     * Multiple copy regions may target different mip levels,
     * array layers, or image regions.
     *
     * The source buffer and destination image are borrowed and
     * must remain valid until the recorded GPU work completes.
     *
     * The copy regions are borrowed for command recording and
     * must remain valid for the duration of the recording call.
     *
     * This structure does not manage image layout transitions,
     * queue submission, or synchronization.
     */
    struct BufferToImageCopyRequest {
        /** Source buffer containing the data to copy. */
        const VulkanBuffer &source;

        /** Destination image receiving the copied data. */
        const VulkanImage &destination;

        /** Layout required for the destination image during copying. */
        VkImageLayout destination_layout{VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL};

        /** Regions describing the image subresources and buffer offsets. */
        std::span<const VkBufferImageCopy2> regions;
    };

    /**
     * @brief Describes an image-to-buffer copy operation.
     *
     * Identifies the source image, destination buffer, source image layout,
     * and image regions to copy.
     *
     * Supports borrowed Vulkan image handles, including swapchain images.
     * The source image and destination buffer must remain valid until the
     * recorded GPU work completes.
     *
     * The copy regions are borrowed for command recording and must remain
     * valid for the duration of the recording call.
     *
     * This structure does not manage image layout transitions, queue
     * submission, synchronization, or resource ownership.
     */
    struct ImageToBufferCopyRequest {
        /** Source image containing the data to copy. */
        VkImage source{VK_NULL_HANDLE};

        /** Destination buffer receiving the copied image data. */
        const VulkanBuffer &destination;

        /** Layout required for the source image during copying. */
        VkImageLayout source_layout{VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL};

        /** Regions describing the image subresources and buffer offsets. */
        std::span<const VkBufferImageCopy2> regions;
    };

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

    /**
     * @brief Records a buffer-to-image copy using Vulkan synchronization2-era
     * copy commands.
     *
     * Records the requested regions using vkCmdCopyBufferToImage2.
     * Does not submit the command buffer or transition image layouts.
     *
     * @param command_buffer Command buffer receiving the copy command.
     * @param request Source, destination, layout, and regions to copy.
     *
     * @pre command_buffer must be a valid recording command buffer.
     * @pre The source buffer must support transfer-source usage.
     * @pre The destination image must support transfer-destination usage.
     * @pre The destination image must already be in the requested layout.
     * @pre The request must contain at least one valid copy region.
     * @pre All copy regions must satisfy Vulkan's buffer-to-image copy
     * requirements.
     */
    auto recordBufferToImageCopy(VkCommandBuffer command_buffer,
                                 const BufferToImageCopyRequest &request)
        -> void;

    /**
     * @brief Records an image-to-buffer copy using Vulkan synchronization2-era
     * copy commands.
     *
     * Records the requested regions using vkCmdCopyImageToBuffer2.
     * Does not submit the command buffer or transition image layouts.
     *
     * @param command_buffer Command buffer receiving the copy command.
     * @param request Source image, destination buffer, layout, and copy
     * regions.
     *
     * @pre command_buffer must be a valid recording command buffer.
     * @pre The source image must be valid and support transfer-source usage.
     * @pre The destination buffer must support transfer-destination usage.
     * @pre The source image must already be in the requested layout.
     * @pre The request must contain at least one valid copy region.
     * @pre Copy regions must fit the source image and destination buffer and
     * satisfy Vulkan's image-to-buffer copy requirements.
     * @pre The submitting queue family must have access to both resources.
     */
    auto recordImageToBufferCopy(VkCommandBuffer command_buffer,
                                 const ImageToBufferCopyRequest &request)
        -> void;
} // namespace SNE::Engine::Renderer::Vulkan
