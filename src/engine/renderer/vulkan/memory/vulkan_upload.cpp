#include "vulkan_upload.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/gpu_memory_usage.hpp"
#include "engine/renderer/vulkan/command/vulkan_image_barrier.hpp"
#include "engine/renderer/vulkan/command/vulkan_transfer_commands.hpp"
#include "vulkan_byte_range.hpp"
#include <array>
#include <limits>

namespace SNE::Engine::Renderer::Vulkan {
    auto uploadBufferData(const VulkanMemoryAllocator &allocator,
                          VulkanImmediateSubmission &immediate_submission,
                          const VulkanBuffer &destination,
                          std::span<const std::byte> bytes,
                          VkDeviceSize destination_offset) -> void {
        if (bytes.empty()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "uploadBufferData requires non-empty data");
        }

        const auto bytes_size = static_cast<VkDeviceSize>(bytes.size());

        const ByteRange upload_range{
            .offset = destination_offset,
            .size = bytes_size,
        };

        if (!upload_range.fitsWithin(destination.size())) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "uploadBufferData range exceeds the destination buffer size");
        }

        VulkanBufferCreateInfo buffer_create_info{};
        buffer_create_info.size = bytes_size;
        buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buffer_create_info.memory_usage = GpuMemoryUsage::Upload;

        VulkanBuffer temporary_upload_buffer =
            VulkanBuffer(allocator, buffer_create_info);
        temporary_upload_buffer.write(bytes);

        immediate_submission.execute(
            [&](VkCommandBuffer command_buffer) -> void {
                BufferCopyRequest copy_request{
                    .source = temporary_upload_buffer,
                    .destination = destination,
                    .source_offset = 0U,
                    .destination_offset = destination_offset,
                    .size = bytes_size,
                };

                recordBufferCopy(command_buffer, copy_request);
            });
    }

    auto uploadImageData(const VulkanMemoryAllocator &allocator,
                         VulkanImmediateSubmission &immediate_submission,
                         const VulkanImage &destination,
                         std::span<const std::byte> pixels) -> void {
        if (pixels.empty()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "uploadImageData requires non-empty pixel data");
        }

        const VulkanImageCreateInfo &destination_info =
            destination.createInfo();

        if (destination_info.image_type != VK_IMAGE_TYPE_2D) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "uploadImageData requires a 2D destination image");
        }

        if (destination_info.format != VK_FORMAT_R8G8B8A8_UNORM) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "uploadImageData requires an RGBA8 UNORM destination image");
        }

        if (destination_info.samples != VK_SAMPLE_COUNT_1_BIT) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "uploadImageData requires a single-sampled destination image");
        }

        if (destination_info.mip_levels != 1U ||
            destination_info.array_layers != 1U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "uploadImageData requires one mip level and one array layer");
        }

        if ((destination.usage() & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "uploadImageData requires transfer-destination image usage");
        }

        constexpr std::size_t kBytesPerPixel = 4U;

        const auto image_width =
            static_cast<std::size_t>(destination_info.extent.width);

        const auto image_height =
            static_cast<std::size_t>(destination_info.extent.height);

        const std::size_t maximum_size =
            std::numeric_limits<std::size_t>::max();

        if (image_width > maximum_size / image_height) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "uploadImageData required pixel size exceeds size_t");
        }

        const std::size_t pixel_count = image_width * image_height;

        if (pixel_count > maximum_size / kBytesPerPixel) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "uploadImageData required pixel size exceeds size_t");
        }

        const std::size_t required_pixel_size = pixel_count * kBytesPerPixel;

        if (pixels.size() != required_pixel_size) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "uploadImageData pixel data size does not match image "
                "dimensions");
        }

        const VulkanBufferCreateInfo staging_buffer_info{
            .size = static_cast<VkDeviceSize>(required_pixel_size),
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .memory_usage = GpuMemoryUsage::Upload,
        };

        VulkanBuffer staging_buffer{allocator, staging_buffer_info};
        staging_buffer.write(pixels);

        VkImageMemoryBarrier2 transfer_barrier{};
        transfer_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        transfer_barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
        transfer_barrier.srcAccessMask = VK_ACCESS_2_NONE;
        transfer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        transfer_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        transfer_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        transfer_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        transfer_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        transfer_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        transfer_barrier.image = destination.nativeHandle();
        transfer_barrier.subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0U,
            .levelCount = 1U,
            .baseArrayLayer = 0U,
            .layerCount = 1U,
        };

        VkBufferImageCopy2 copy_region{};
        copy_region.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2;
        copy_region.bufferOffset = 0U;
        copy_region.imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0U,
            .baseArrayLayer = 0U,
            .layerCount = 1U,
        };
        copy_region.imageExtent = destination_info.extent;

        const std::array<VkBufferImageCopy2, 1U> regions{copy_region};

        const BufferToImageCopyRequest copy_request{
            .source = staging_buffer,
            .destination = destination,
            .destination_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .regions = regions,
        };

        immediate_submission.execute(
            [&](VkCommandBuffer command_buffer) -> void {
                recordImageBarrier(command_buffer, transfer_barrier);
                recordBufferToImageCopy(command_buffer, copy_request);
            });
    }
} // namespace SNE::Engine::Renderer::Vulkan
