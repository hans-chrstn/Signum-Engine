#pragma once

#include "engine/renderer/vulkan/command/vulkan_immediate_submission.hpp"
#include "vulkan_buffer.hpp"
#include "vulkan_image.hpp"
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

    /**
     * @brief Uploads tightly packed RGBA8 pixel data to a Vulkan image.
     *
     * Creates a temporary staging buffer, copies the supplied pixels
     * into it, transitions the destination image from UNDEFINED to
     * TRANSFER_DST_OPTIMAL, and records a buffer-to-image copy.
     *
     * Submission is synchronous. The temporary staging buffer remains
     * valid until GPU execution completes.
     *
     * The destination remains in VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL.
     * The caller must transition it before using it for another purpose.
     *
     * @param allocator Allocator used to create the staging buffer.
     * @param immediate_submission Context used for synchronous GPU submission.
     * @param destination Newly created image receiving the pixel data.
     * @param pixels Tightly packed RGBA8 pixel data.
     *
     * @pre pixels must not be empty.
     * @pre destination must be a single-sampled 2D image using
     * VK_FORMAT_R8G8B8A8_UNORM.
     * @pre destination must have one mip level and one array layer.
     * @pre destination must support transfer-destination usage.
     * @pre pixels must contain exactly the required image data.
     * @pre destination must initially have VK_IMAGE_LAYOUT_UNDEFINED.
     */
    auto uploadImageData(const VulkanMemoryAllocator &allocator,
                         VulkanImmediateSubmission &immediate_submission,
                         const VulkanImage &destination,
                         std::span<const std::byte> pixels) -> void;

} // namespace SNE::Engine::Renderer::Vulkan
