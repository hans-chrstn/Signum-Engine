
#include "../vulkan_integration_test_fixture.hpp"
#include "engine/renderer/gpu_memory_usage.hpp"
#include "engine/renderer/vulkan/command/vulkan_image_barrier.hpp"
#include "engine/renderer/vulkan/command/vulkan_transfer_commands.hpp"
#include "engine/renderer/vulkan/memory/vulkan_buffer.hpp"
#include "engine/renderer/vulkan/memory/vulkan_image.hpp"
#include "engine/renderer/vulkan/memory/vulkan_upload.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>
#include <span>
#include <vulkan/vulkan.h>

namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = Renderer::Vulkan;

TEST_F(VulkanIntegrationTest,
       RecordBufferCopyRejectsSourceWithoutTransferSourceUsage) {
    Vulkan::VulkanMemoryAllocator &memory_allocator = memoryAllocator();

    const Vulkan::VulkanBufferCreateInfo source_create_info{
        .size = 16U,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Device,
    };

    const Vulkan::VulkanBufferCreateInfo destination_create_info{
        .size = 16U,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Device,
    };

    const Vulkan::VulkanBuffer source{memory_allocator, source_create_info};
    const Vulkan::VulkanBuffer destination{memory_allocator,
                                           destination_create_info};

    std::byte dummy{};
    auto *command_buffer = reinterpret_cast<VkCommandBuffer>(&dummy);

    const Vulkan::BufferCopyRequest copy_request{
        .source = source,
        .destination = destination,
        .source_offset = 0U,
        .destination_offset = 0U,
        .size = 4U,
    };

    ASSERT_DEATH(Vulkan::recordBufferCopy(command_buffer, copy_request),
                 "recordBufferCopy requires the source buffer to support "
                 "transfer-source usage");
}

TEST_F(VulkanIntegrationTest,
       RecordBufferCopyRejectsDestinationWithoutTransferDestinationUsage) {
    Vulkan::VulkanMemoryAllocator &memory_allocator = memoryAllocator();

    const Vulkan::VulkanBufferCreateInfo source_create_info{
        .size = 16U,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Device,
    };

    const Vulkan::VulkanBufferCreateInfo destination_create_info{
        .size = 16U,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Device,
    };

    const Vulkan::VulkanBuffer source{memory_allocator, source_create_info};
    const Vulkan::VulkanBuffer destination{memory_allocator,
                                           destination_create_info};

    std::byte dummy{};
    auto *command_buffer = reinterpret_cast<VkCommandBuffer>(&dummy);

    const Vulkan::BufferCopyRequest copy_request{
        .source = source,
        .destination = destination,
        .source_offset = 0U,
        .destination_offset = 0U,
        .size = 4U,
    };

    ASSERT_DEATH(Vulkan::recordBufferCopy(command_buffer, copy_request),
                 "recordBufferCopy requires the destination buffer to support "
                 "transfer-destination usage");
}

TEST_F(VulkanIntegrationTest,
       RecordBufferCopyRejectsOverlappingSameBufferRanges) {
    Vulkan::VulkanMemoryAllocator &memory_allocator = memoryAllocator();

    const Vulkan::VulkanBufferCreateInfo buffer_create_info{
        .size = 16U,
        .usage =
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Device,
    };

    const Vulkan::VulkanBuffer buffer{memory_allocator, buffer_create_info};

    std::byte dummy{};
    auto *command_buffer = reinterpret_cast<VkCommandBuffer>(&dummy);

    const Vulkan::BufferCopyRequest copy_request{
        .source = buffer,
        .destination = buffer,
        .source_offset = 0U,
        .destination_offset = 2U,
        .size = 4U,
    };

    ASSERT_DEATH(
        Vulkan::recordBufferCopy(command_buffer, copy_request),
        "recordBufferCopy requires non-overlapping ranges when copying "
        "within the same buffer");
}

TEST_F(VulkanIntegrationTest,
       RecordBufferCopyAllowsNonOverlappingSameBufferRanges) {
    Vulkan::VulkanMemoryAllocator &memory_allocator = memoryAllocator();

    const Vulkan::VulkanBufferCreateInfo buffer_create_info{
        .size = 8U,
        .usage =
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Device,
    };

    const Vulkan::VulkanBuffer buffer{memory_allocator, buffer_create_info};

    const Vulkan::BufferCopyRequest copy_request{
        .source = buffer,
        .destination = buffer,
        .source_offset = 0U,
        .destination_offset = 4U,
        .size = 4U,
    };

    EXPECT_NO_THROW(immediateSubmission().execute(
        [&](VkCommandBuffer command_buffer) -> void {
            Vulkan::recordBufferCopy(command_buffer, copy_request);
        }));
}

TEST_F(VulkanIntegrationTest, RecordsBufferToImageCopy) {
    constexpr std::uint32_t kImageExtent = 4U;
    constexpr std::size_t kBytesPerPixel = 4U;
    constexpr std::size_t kBufferSize =
        static_cast<std::size_t>(kImageExtent) * kImageExtent * kBytesPerPixel;

    const Vulkan::VulkanBufferCreateInfo buffer_info{
        .size = static_cast<VkDeviceSize>(kBufferSize),
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Upload,
    };

    const Vulkan::VulkanImageCreateInfo image_info{
        .extent =
            {
                .width = kImageExtent,
                .height = kImageExtent,
                .depth = 1U,
            },
        .image_type = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    };

    Vulkan::VulkanBuffer source{memoryAllocator(), buffer_info};
    const Vulkan::VulkanImage destination{memoryAllocator(), image_info};

    const std::array<std::byte, kBufferSize> pixels{};
    source.write(std::span<const std::byte>{pixels});

    VkBufferImageCopy2 region{};
    region.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2;
    region.bufferOffset = 0U;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0U;
    region.imageSubresource.baseArrayLayer = 0U;
    region.imageSubresource.layerCount = 1U;
    region.imageExtent = {
        .width = kImageExtent,
        .height = kImageExtent,
        .depth = 1U,
    };

    const std::array<VkBufferImageCopy2, 1U> regions{region};

    const Vulkan::BufferToImageCopyRequest request{
        .source = source,
        .destination = destination,
        .destination_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .regions = regions,
    };

    VkImageMemoryBarrier2 barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
    barrier.srcAccessMask = VK_ACCESS_2_NONE;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = destination.nativeHandle();
    barrier.subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0U,
        .levelCount = 1U,
        .baseArrayLayer = 0U,
        .layerCount = 1U,
    };

    EXPECT_NO_THROW(immediateSubmission().execute(
        [&](VkCommandBuffer command_buffer) -> void {
            Vulkan::recordImageBarrier(command_buffer, barrier);
            Vulkan::recordBufferToImageCopy(command_buffer, request);
        }));
}

TEST_F(VulkanIntegrationTest, RejectsInvalidBufferToImageCopyRequests) {
    constexpr VkDeviceSize kBufferSize = 64U;

    const Vulkan::VulkanBufferCreateInfo valid_buffer_info{
        .size = kBufferSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Device,
    };

    const Vulkan::VulkanBufferCreateInfo invalid_buffer_info{
        .size = kBufferSize,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Device,
    };

    const Vulkan::VulkanImageCreateInfo valid_image_info{
        .extent =
            {
                .width = 4U,
                .height = 4U,
                .depth = 1U,
            },
        .image_type = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    };

    Vulkan::VulkanImageCreateInfo invalid_image_info = valid_image_info;
    invalid_image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    const Vulkan::VulkanBuffer valid_source{memoryAllocator(),
                                            valid_buffer_info};
    const Vulkan::VulkanBuffer invalid_source{memoryAllocator(),
                                              invalid_buffer_info};
    const Vulkan::VulkanImage valid_destination{memoryAllocator(),
                                                valid_image_info};
    const Vulkan::VulkanImage invalid_destination{memoryAllocator(),
                                                  invalid_image_info};

    std::byte dummy{};
    auto *command_buffer = reinterpret_cast<VkCommandBuffer>(&dummy);

    const Vulkan::BufferToImageCopyRequest valid_resources{
        .source = valid_source,
        .destination = valid_destination,
        .regions = {},
    };

    ASSERT_DEATH(
        Vulkan::recordBufferToImageCopy(VK_NULL_HANDLE, valid_resources),
        "recordBufferToImageCopy requires a valid command buffer");

    const Vulkan::BufferToImageCopyRequest invalid_source_request{
        .source = invalid_source,
        .destination = valid_destination,
        .regions = {},
    };

    ASSERT_DEATH(
        Vulkan::recordBufferToImageCopy(command_buffer, invalid_source_request),
        "recordBufferToImageCopy requires the source buffer to support "
        "transfer-source usage");

    const Vulkan::BufferToImageCopyRequest invalid_destination_request{
        .source = valid_source,
        .destination = invalid_destination,
        .regions = {},
    };

    ASSERT_DEATH(
        Vulkan::recordBufferToImageCopy(command_buffer,
                                        invalid_destination_request),
        "recordBufferToImageCopy requires the destination image to support "
        "transfer-destination usage");

    ASSERT_DEATH(
        Vulkan::recordBufferToImageCopy(command_buffer, valid_resources),
        "recordBufferToImageCopy requires at least one copy region");
}

TEST_F(VulkanIntegrationTest, RecordBufferToImageCopyRejectsInvalidRegions) {
    constexpr std::uint32_t kImageExtent = 4U;
    constexpr VkDeviceSize kBufferSize = 64U;

    const Vulkan::VulkanBufferCreateInfo buffer_info{
        .size = kBufferSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Device,
    };

    const Vulkan::VulkanImageCreateInfo image_info{
        .extent =
            {
                .width = kImageExtent,
                .height = kImageExtent,
                .depth = 1U,
            },
        .image_type = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    };

    const Vulkan::VulkanBuffer source{memoryAllocator(), buffer_info};
    const Vulkan::VulkanImage destination{memoryAllocator(), image_info};

    VkBufferImageCopy2 region{};
    region.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2;
    region.imageExtent = {
        .width = kImageExtent,
        .height = kImageExtent,
        .depth = 1U,
    };
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1U;

    std::array<VkBufferImageCopy2, 1U> regions{region};

    const Vulkan::BufferToImageCopyRequest request{
        .source = source,
        .destination = destination,
        .regions = regions,
    };

    std::byte dummy{};
    auto *command_buffer = reinterpret_cast<VkCommandBuffer>(&dummy);

    regions[0].sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

    ASSERT_DEATH(
        Vulkan::recordBufferToImageCopy(command_buffer, request),
        "recordBufferToImageCopy requires a valid copy region structure type");

    regions[0].sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2;
    regions[0].imageExtent.width = 0U;

    ASSERT_DEATH(
        Vulkan::recordBufferToImageCopy(command_buffer, request),
        "recordBufferToImageCopy requires a non-zero image copy extent");

    regions[0].imageExtent.width = kImageExtent;
    regions[0].bufferRowLength = kImageExtent - 1U;

    ASSERT_DEATH(Vulkan::recordBufferToImageCopy(command_buffer, request),
                 "recordBufferToImageCopy buffer row length is smaller than "
                 "the copy width");

    regions[0].bufferRowLength = 0U;
    regions[0].bufferImageHeight = kImageExtent - 1U;

    ASSERT_DEATH(Vulkan::recordBufferToImageCopy(command_buffer, request),
                 "recordBufferToImageCopy buffer image height is smaller "
                 "than the copy height");

    regions[0].bufferImageHeight = 0U;
    regions[0].imageSubresource.layerCount = 0U;

    ASSERT_DEATH(
        Vulkan::recordBufferToImageCopy(command_buffer, request),
        "recordBufferToImageCopy requires at least one image array layer");

    regions[0].imageSubresource.layerCount = 1U;
    regions[0].imageSubresource.aspectMask = 0U;

    ASSERT_DEATH(
        Vulkan::recordBufferToImageCopy(command_buffer, request),
        "recordBufferToImageCopy requires exactly one image aspect per region");

    regions[0].imageSubresource.aspectMask =
        VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;

    ASSERT_DEATH(
        Vulkan::recordBufferToImageCopy(command_buffer, request),
        "recordBufferToImageCopy requires exactly one image aspect per region");

    regions[0].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    regions[0].imageSubresource.mipLevel = 1U;

    ASSERT_DEATH(
        Vulkan::recordBufferToImageCopy(command_buffer, request),
        "recordBufferToImageCopy mip level exceeds the destination image");

    regions[0].imageSubresource.mipLevel = 0U;
    regions[0].imageSubresource.baseArrayLayer = 1U;

    ASSERT_DEATH(
        Vulkan::recordBufferToImageCopy(command_buffer, request),
        "recordBufferToImageCopy array layer range exceeds the destination "
        "image");

    regions[0].imageSubresource.baseArrayLayer = 0U;
    regions[0].imageSubresource.layerCount = 2U;

    ASSERT_DEATH(
        Vulkan::recordBufferToImageCopy(command_buffer, request),
        "recordBufferToImageCopy array layer range exceeds the destination "
        "image");

    regions[0].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    regions[0].imageSubresource.mipLevel = 0U;
    regions[0].imageSubresource.baseArrayLayer = 0U;
    regions[0].imageSubresource.layerCount = 1U;

    regions[0].imageOffset = {
        .x = -1,
        .y = 0,
        .z = 0,
    };

    ASSERT_DEATH(Vulkan::recordBufferToImageCopy(command_buffer, request),
                 "recordBufferToImageCopy requires non-negative image offsets");

    regions[0].imageOffset = {
        .x = 1,
        .y = 0,
        .z = 0,
    };

    ASSERT_DEATH(Vulkan::recordBufferToImageCopy(command_buffer, request),
                 "recordBufferToImageCopy region exceeds the destination mip "
                 "dimensions");

    regions[0].imageOffset = {
        .x = 0,
        .y = 1,
        .z = 0,
    };

    ASSERT_DEATH(Vulkan::recordBufferToImageCopy(command_buffer, request),
                 "recordBufferToImageCopy region exceeds the destination mip "
                 "dimensions");

    regions[0].imageOffset = {
        .x = 0,
        .y = 0,
        .z = 1,
    };

    ASSERT_DEATH(Vulkan::recordBufferToImageCopy(command_buffer, request),
                 "recordBufferToImageCopy region exceeds the destination mip "
                 "dimensions");
}

TEST_F(VulkanIntegrationTest,
       RecordBufferToImageCopyRejectsCopyOutsideSelectedMip) {
    constexpr std::uint32_t kImageExtent = 4U;
    constexpr std::uint32_t kMipLevels = 2U;
    constexpr VkDeviceSize kBufferSize = 16U;

    const Vulkan::VulkanBufferCreateInfo buffer_info{
        .size = kBufferSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Device,
    };

    const Vulkan::VulkanImageCreateInfo image_info{
        .extent =
            {
                .width = kImageExtent,
                .height = kImageExtent,
                .depth = 1U,
            },
        .image_type = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .mip_levels = kMipLevels,
    };

    const Vulkan::VulkanBuffer source{memoryAllocator(), buffer_info};
    const Vulkan::VulkanImage destination{memoryAllocator(), image_info};

    VkBufferImageCopy2 region{};
    region.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 1U;
    region.imageSubresource.baseArrayLayer = 0U;
    region.imageSubresource.layerCount = 1U;
    region.imageExtent = {
        .width = 2U,
        .height = 2U,
        .depth = 1U,
    };

    std::array<VkBufferImageCopy2, 1U> regions{region};

    const Vulkan::BufferToImageCopyRequest request{
        .source = source,
        .destination = destination,
        .regions = regions,
    };

    std::byte dummy{};
    auto *command_buffer = reinterpret_cast<VkCommandBuffer>(&dummy);

    regions[0].imageOffset = {
        .x = 1,
        .y = 0,
        .z = 0,
    };

    ASSERT_DEATH(Vulkan::recordBufferToImageCopy(command_buffer, request),
                 "recordBufferToImageCopy region exceeds the destination mip "
                 "dimensions");

    regions[0].imageOffset = {
        .x = 0,
        .y = 1,
        .z = 0,
    };

    ASSERT_DEATH(Vulkan::recordBufferToImageCopy(command_buffer, request),
                 "recordBufferToImageCopy region exceeds the destination mip "
                 "dimensions");
}

TEST_F(VulkanIntegrationTest, UploadsImageData) {
    constexpr std::uint32_t kImageExtent = 4U;
    constexpr std::size_t kBytesPerPixel = 4U;
    constexpr std::size_t kPixelDataSize =
        static_cast<std::size_t>(kImageExtent) * kImageExtent * kBytesPerPixel;

    const Vulkan::VulkanImageCreateInfo image_info{
        .extent =
            {
                .width = kImageExtent,
                .height = kImageExtent,
                .depth = 1U,
            },
        .image_type = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    };

    const Vulkan::VulkanImage destination{memoryAllocator(), image_info};
    const std::array<std::byte, kPixelDataSize> pixels{};

    EXPECT_NO_THROW(Vulkan::uploadImageData(
        memoryAllocator(), immediateSubmission(), destination, pixels));
}

TEST_F(VulkanIntegrationTest, UploadsSrgbImageData) {
    constexpr std::uint32_t kImageExtent = 4U;
    constexpr std::size_t kBytesPerPixel = 4U;
    constexpr std::size_t kPixelDataSize =
        static_cast<std::size_t>(kImageExtent) * kImageExtent * kBytesPerPixel;

    const Vulkan::VulkanImageCreateInfo image_info{
        .extent =
            {
                .width = kImageExtent,
                .height = kImageExtent,
                .depth = 1U,
            },
        .image_type = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_SRGB,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    };

    const Vulkan::VulkanImage destination{memoryAllocator(), image_info};
    const std::array<std::byte, kPixelDataSize> pixels{};

    EXPECT_NO_THROW(Vulkan::uploadImageData(
        memoryAllocator(), immediateSubmission(), destination, pixels));
}

TEST_F(VulkanIntegrationTest, UploadImageDataRejectsInvalidPixelSizes) {
    const Vulkan::VulkanImageCreateInfo image_info{
        .extent =
            {
                .width = 4U,
                .height = 4U,
                .depth = 1U,
            },
        .image_type = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    };

    const Vulkan::VulkanImage destination{memoryAllocator(), image_info};

    const std::span<const std::byte> empty_pixels{};

    ASSERT_DEATH(Vulkan::uploadImageData(memoryAllocator(),
                                         immediateSubmission(), destination,
                                         empty_pixels),
                 "uploadImageData requires non-empty pixel data");

    const std::array<std::byte, 63U> undersized_pixels{};

    ASSERT_DEATH(
        Vulkan::uploadImageData(memoryAllocator(), immediateSubmission(),
                                destination, undersized_pixels),
        "uploadImageData pixel data size does not match image dimensions");
}

TEST_F(VulkanIntegrationTest, UploadImageDataRejectsOversizedPixelData) {
    Vulkan::VulkanImageCreateInfo image_info{};
    image_info.extent = {
        .width = 4U,
        .height = 4U,
        .depth = 1U,
    };
    image_info.image_type = VK_IMAGE_TYPE_2D;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    const Vulkan::VulkanImage destination{memoryAllocator(), image_info};
    const std::array<std::byte, 65U> pixels{};

    ASSERT_DEATH(
        Vulkan::uploadImageData(memoryAllocator(), immediateSubmission(),
                                destination, pixels),
        "uploadImageData pixel data size does not match image dimensions");
}

TEST_F(VulkanIntegrationTest, UploadImageDataRejectsUnsupportedDestinations) {
    Vulkan::VulkanImageCreateInfo valid_image_info{};
    valid_image_info.extent = {
        .width = 4U,
        .height = 4U,
        .depth = 1U,
    };
    valid_image_info.image_type = VK_IMAGE_TYPE_2D;
    valid_image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    valid_image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    const std::array<std::byte, 64U> pixels{};

    Vulkan::VulkanImageCreateInfo invalid_type_info = valid_image_info;
    invalid_type_info.image_type = VK_IMAGE_TYPE_1D;
    invalid_type_info.extent.height = 1U;

    const Vulkan::VulkanImage invalid_type{memoryAllocator(),
                                           invalid_type_info};

    ASSERT_DEATH(Vulkan::uploadImageData(memoryAllocator(),
                                         immediateSubmission(), invalid_type,
                                         pixels),
                 "uploadImageData requires a 2D destination image");

    Vulkan::VulkanImageCreateInfo invalid_format_info = valid_image_info;
    invalid_format_info.format = VK_FORMAT_R8_UNORM;

    const Vulkan::VulkanImage invalid_format{memoryAllocator(),
                                             invalid_format_info};

    ASSERT_DEATH(
        Vulkan::uploadImageData(memoryAllocator(), immediateSubmission(),
                                invalid_format, pixels),
        "uploadImageData requires an RGBA8 UNORM or sRGB destination image");

    Vulkan::VulkanImageCreateInfo invalid_layers_info = valid_image_info;
    invalid_layers_info.array_layers = 2U;

    const Vulkan::VulkanImage invalid_layers{memoryAllocator(),
                                             invalid_layers_info};

    ASSERT_DEATH(Vulkan::uploadImageData(memoryAllocator(),
                                         immediateSubmission(), invalid_layers,
                                         pixels),
                 "uploadImageData requires exactly one array layer");

    Vulkan::VulkanImageCreateInfo invalid_usage_info = valid_image_info;
    invalid_usage_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    const Vulkan::VulkanImage invalid_usage{memoryAllocator(),
                                            invalid_usage_info};

    ASSERT_DEATH(Vulkan::uploadImageData(memoryAllocator(),
                                         immediateSubmission(), invalid_usage,
                                         pixels),
                 "uploadImageData requires transfer-destination image usage");
}

TEST_F(VulkanIntegrationTest, UploadsCompleteUnormMipChain) {
    constexpr std::uint32_t kImageExtent = 4U;
    constexpr std::uint32_t kMipLevels = 3U;
    constexpr std::size_t kBytesPerPixel = 4U;

    constexpr auto kExtent = static_cast<std::size_t>(kImageExtent);
    constexpr std::size_t kHalfExtent = kExtent / 2U;

    constexpr std::size_t kPixelCount =
        (kExtent * kExtent) + (kHalfExtent * kHalfExtent) + std::size_t{1U};

    constexpr std::size_t kPixelDataSize = kPixelCount * kBytesPerPixel;

    Vulkan::VulkanImageCreateInfo image_info{};
    image_info.extent = {
        .width = kImageExtent,
        .height = kImageExtent,
        .depth = 1U,
    };
    image_info.image_type = VK_IMAGE_TYPE_2D;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_info.mip_levels = kMipLevels;

    const Vulkan::VulkanImage destination{memoryAllocator(), image_info};

    const std::array<std::byte, kPixelDataSize> pixels{};

    EXPECT_NO_THROW(Vulkan::uploadImageData(
        memoryAllocator(), immediateSubmission(), destination, pixels));
}

TEST_F(VulkanIntegrationTest, UploadsNonSquareSrgbMipChain) {
    constexpr std::uint32_t kWidth = 4U;
    constexpr std::uint32_t kHeight = 2U;
    constexpr std::uint32_t kMipLevels = 3U;
    constexpr std::size_t kBytesPerPixel = 4U;

    constexpr auto kWidthSize = static_cast<std::size_t>(kWidth);
    constexpr auto kHeightSize = static_cast<std::size_t>(kHeight);

    constexpr std::size_t kPixelCount =
        (kWidthSize * kHeightSize) + ((kWidthSize / 2U) * (kHeightSize / 2U)) +
        std::size_t{1U};

    constexpr std::size_t kPixelDataSize = kPixelCount * kBytesPerPixel;

    Vulkan::VulkanImageCreateInfo image_info{};
    image_info.extent = {
        .width = kWidth,
        .height = kHeight,
        .depth = 1U,
    };
    image_info.image_type = VK_IMAGE_TYPE_2D;
    image_info.format = VK_FORMAT_R8G8B8A8_SRGB;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_info.mip_levels = kMipLevels;

    const Vulkan::VulkanImage destination{memoryAllocator(), image_info};

    const std::array<std::byte, kPixelDataSize> pixels{};

    EXPECT_NO_THROW(Vulkan::uploadImageData(
        memoryAllocator(), immediateSubmission(), destination, pixels));
}

TEST_F(VulkanIntegrationTest, RejectsIncompleteMipChainData) {
    constexpr std::uint32_t kImageExtent = 4U;
    constexpr std::uint32_t kMipLevels = 3U;
    constexpr std::size_t kBytesPerPixel = 4U;

    constexpr auto kExtent = static_cast<std::size_t>(kImageExtent);

    constexpr std::size_t kBaseLevelSize = kExtent * kExtent * kBytesPerPixel;

    Vulkan::VulkanImageCreateInfo image_info{};
    image_info.extent = {
        .width = kImageExtent,
        .height = kImageExtent,
        .depth = 1U,
    };
    image_info.image_type = VK_IMAGE_TYPE_2D;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_info.mip_levels = kMipLevels;

    const Vulkan::VulkanImage destination{memoryAllocator(), image_info};

    const std::array<std::byte, kBaseLevelSize> pixels{};

    ASSERT_DEATH(Vulkan::uploadImageData(memoryAllocator(),
                                         immediateSubmission(), destination,
                                         pixels),
                 "pixel data size does not match image dimensions");
}
