#pragma once
#include "engine/renderer/vulkan/command/vulkan_immediate_submission.hpp"
#include "vulkan_buffer.hpp"
#include "vulkan_memory_allocator.hpp"
#include <cstddef>
#include <span>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Uploads CPU data into a Vulkan buffer through a staging transfer.
     *
     * Creates a temporary host-visible upload buffer, writes the supplied bytes
     * into it, and synchronously copies those bytes into the destination
     * buffer.
     *
     * The temporary upload buffer remains alive until the GPU transfer has
     * completed.
     *
     * The VulkanMemoryAllocator is borrowed and must remain valid for the
     * duration of this operation.
     *
     * @param allocator Engine-owned Vulkan memory allocator used to create the
     * temporary upload buffer.
     * @param immediate_submission Submission context used to execute the
     * transfer.
     * @param destination Buffer that receives the uploaded data.
     * @param bytes CPU bytes to upload.
     * @param destination_offset Byte offset within the destination buffer at
     * which the uploaded data is written.
     *
     * @pre bytes must not be empty.
     * @pre The destination range must fit within the destination buffer.
     * @pre destination must have been created with
     * VK_BUFFER_USAGE_TRANSFER_DST_BIT.
     */
    auto uploadBufferData(const VulkanMemoryAllocator &allocator,
                          VulkanImmediateSubmission &immediate_submission,
                          const VulkanBuffer &destination,
                          std::span<const std::byte> bytes,
                          VkDeviceSize destination_offset = 0U) -> void;
} // namespace SNE::Engine::Renderer::Vulkan
