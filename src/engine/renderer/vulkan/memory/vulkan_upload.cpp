#include "vulkan_upload.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/gpu_memory_usage.hpp"
#include "engine/renderer/vulkan/command/vulkan_image_barrier.hpp"
#include "engine/renderer/vulkan/command/vulkan_transfer_commands.hpp"
#include "vulkan_byte_range.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

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

        if (destination_info.format != VK_FORMAT_R8G8B8A8_UNORM &&
            destination_info.format != VK_FORMAT_R8G8B8A8_SRGB) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "uploadImageData requires an RGBA8 UNORM or sRGB destination "
                "image");
        }

        if (destination_info.samples != VK_SAMPLE_COUNT_1_BIT) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "uploadImageData requires a single-sampled destination image");
        }

        if (destination_info.array_layers != 1U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "uploadImageData requires exactly one array layer");
        }

        if ((destination.usage() & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "uploadImageData requires transfer-destination image usage");
        }

        constexpr std::size_t kBytesPerPixel = 4U;
        const std::size_t maximum_size =
            std::numeric_limits<std::size_t>::max();

        auto mip_width =
            static_cast<std::size_t>(destination_info.extent.width);
        auto mip_height =
            static_cast<std::size_t>(destination_info.extent.height);

        std::size_t required_pixel_size{};

        for (std::uint32_t level{}; level < destination_info.mip_levels;
             ++level) {
            if (mip_width > maximum_size / mip_height) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "uploadImageData required mip pixel count exceeds size_t");
            }

            const std::size_t mip_pixel_count = mip_width * mip_height;

            if (mip_pixel_count > maximum_size / kBytesPerPixel) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "uploadImageData required mip byte count exceeds size_t");
            }

            const std::size_t mip_byte_count = mip_pixel_count * kBytesPerPixel;

            if (mip_byte_count > maximum_size - required_pixel_size) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "uploadImageData total mip byte count exceeds size_t");
            }

            required_pixel_size += mip_byte_count;

            mip_width = std::max(std::size_t{1U}, mip_width / 2U);
            mip_height = std::max(std::size_t{1U}, mip_height / 2U);
        }

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
            .levelCount = destination_info.mip_levels,
            .baseArrayLayer = 0U,
            .layerCount = 1U,
        };

        std::vector<VkBufferImageCopy2> regions;
        regions.reserve(destination_info.mip_levels);

        mip_width = static_cast<std::size_t>(destination_info.extent.width);
        mip_height = static_cast<std::size_t>(destination_info.extent.height);

        std::size_t buffer_offset{};

        for (std::uint32_t level{}; level < destination_info.mip_levels;
             ++level) {
            VkBufferImageCopy2 region{};
            region.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2;
            region.bufferOffset = static_cast<VkDeviceSize>(buffer_offset);

            region.imageSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = level,
                .baseArrayLayer = 0U,
                .layerCount = 1U,
            };

            region.imageExtent = {
                .width = static_cast<std::uint32_t>(mip_width),
                .height = static_cast<std::uint32_t>(mip_height),
                .depth = 1U,
            };

            regions.push_back(region);

            buffer_offset += mip_width * mip_height * kBytesPerPixel;

            mip_width = std::max(std::size_t{1U}, mip_width / 2U);
            mip_height = std::max(std::size_t{1U}, mip_height / 2U);
        }

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
