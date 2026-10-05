#pragma once

#include "vulkan_buffer.hpp"
#include "vulkan_immediate_submission.hpp"
#include <cstddef>
#include <span>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Reads GPU buffer data back into CPU memory.
     *
     * Creates a temporary host-readable readback buffer, copies the requested
     * range from the source buffer into it, waits for the transfer to complete,
     * and then copies the resulting bytes into the supplied output span.
     *
     * The temporary readback buffer remains alive until the GPU transfer has
     * completed and its contents have been copied into CPU memory.
     *
     * @param allocator VMA allocator used to create the temporary readback
     * buffer.
     * @param immediate_submission Submission context used to execute the
     * transfer.
     * @param source Buffer whose contents are read back.
     * @param output Destination span that receives the copied bytes.
     * @param source_offset Byte offset within the source buffer at which
     * reading begins.
     *
     * @pre allocator must be a valid VMA allocator.
     * @pre output must not be empty.
     * @pre The requested source range must fit entirely within the source
     * buffer.
     * @pre source must have been created with
     * VK_BUFFER_USAGE_TRANSFER_SRC_BIT.
     */
    auto readBufferData(VmaAllocator allocator,
                        VulkanImmediateSubmission &immediate_submission,
                        const VulkanBuffer &source, std::span<std::byte> output,
                        VkDeviceSize source_offset = 0U) -> void;
} // namespace SNE::Engine::Renderer::Vulkan
