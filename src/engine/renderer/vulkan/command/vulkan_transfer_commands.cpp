#include "vulkan_transfer_commands.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/core/numeric/checked_conversion.hpp"
#include "engine/renderer/vulkan/memory/vulkan_byte_range.hpp"
#include "engine/renderer/vulkan/memory/vulkan_image.hpp"
#include <algorithm>
#include <bit>
#include <cstdint>
#include <optional>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;
namespace Core = SNE::Engine::Core;

namespace {
    auto validateBufferToImageRegion(
        const VkBufferImageCopy2 &region,
        const Vulkan::VulkanImageCreateInfo &destination_info) -> void {
        if (region.sType != VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "recordBufferToImageCopy requires a valid copy region "
                "structure type");
        }

        if (region.imageExtent.width == 0U || region.imageExtent.height == 0U ||
            region.imageExtent.depth == 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "recordBufferToImageCopy requires a non-zero image copy "
                "extent");
        }

        if (region.bufferRowLength != 0U &&
            region.bufferRowLength < region.imageExtent.width) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "recordBufferToImageCopy buffer row length is smaller than "
                "the copy width");
        }

        if (region.bufferImageHeight != 0U &&
            region.bufferImageHeight < region.imageExtent.height) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "recordBufferToImageCopy buffer image height is smaller "
                "than the copy height");
        }

        if (region.imageSubresource.layerCount == 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "recordBufferToImageCopy requires at least one image array "
                "layer");
        }

        if (!std::has_single_bit(region.imageSubresource.aspectMask)) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "recordBufferToImageCopy requires exactly one image aspect "
                "per region");
        }

        if (region.imageSubresource.mipLevel >= destination_info.mip_levels) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "recordBufferToImageCopy mip level exceeds the destination "
                "image");
        }

        const std::uint32_t base_layer = region.imageSubresource.baseArrayLayer;

        const std::uint32_t layer_count = region.imageSubresource.layerCount;

        if (base_layer >= destination_info.array_layers) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "recordBufferToImageCopy array layer range exceeds the "
                "destination image");
        }

        const std::uint32_t remaining_layers =
            destination_info.array_layers - base_layer;

        if (layer_count > remaining_layers) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "recordBufferToImageCopy array layer range exceeds the "
                "destination image");
        }
    }

    auto validateBufferToImageBounds(
        const VkBufferImageCopy2 &region,
        const Vulkan::VulkanImageCreateInfo &destination_info) -> void {
        const VkOffset3D offset = region.imageOffset;

        if (offset.x < 0 || offset.y < 0 || offset.z < 0) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "recordBufferToImageCopy requires non-negative image offsets");
        }

        const std::uint32_t mip_level = region.imageSubresource.mipLevel;

        const VkExtent3D mip_extent{
            .width = std::max(1U, destination_info.extent.width >> mip_level),
            .height = std::max(1U, destination_info.extent.height >> mip_level),
            .depth = std::max(1U, destination_info.extent.depth >> mip_level),
        };

        const auto offset_x = static_cast<std::uint32_t>(offset.x);
        const auto offset_y = static_cast<std::uint32_t>(offset.y);
        const auto offset_z = static_cast<std::uint32_t>(offset.z);

        if (offset_x > mip_extent.width ||
            region.imageExtent.width > mip_extent.width - offset_x ||
            offset_y > mip_extent.height ||
            region.imageExtent.height > mip_extent.height - offset_y ||
            offset_z > mip_extent.depth ||
            region.imageExtent.depth > mip_extent.depth - offset_z) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "recordBufferToImageCopy region exceeds the destination mip "
                "dimensions");
        }
    }

} // namespace

namespace SNE::Engine::Renderer::Vulkan {

    auto recordBufferCopy(VkCommandBuffer command_buffer,
                          const BufferCopyRequest &buffer_copy_request)
        -> void {
        if (command_buffer == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "recordBufferCopy requires a valid command buffer");
        }

        if (buffer_copy_request.size == 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "recordBufferCopy requires a non-zero copy size");
        }

        if ((buffer_copy_request.source.usage() &
             VK_BUFFER_USAGE_TRANSFER_SRC_BIT) == 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "recordBufferCopy requires the source buffer to support "
                "transfer-source usage");
        }

        if ((buffer_copy_request.destination.usage() &
             VK_BUFFER_USAGE_TRANSFER_DST_BIT) == 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "recordBufferCopy requires the destination buffer to support "
                "transfer-destination usage");
        }

        const ByteRange source_range{
            .offset = buffer_copy_request.source_offset,
            .size = buffer_copy_request.size,
        };

        if (!source_range.fitsWithin(buffer_copy_request.source.size())) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "recordBufferCopy source range exceeds the source buffer size");
        }

        const ByteRange destination_range{
            .offset = buffer_copy_request.destination_offset,
            .size = buffer_copy_request.size,
        };

        if (!destination_range.fitsWithin(
                buffer_copy_request.destination.size())) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "recordBufferCopy destination range exceeds the destination "
                "buffer size");
        }

        if (buffer_copy_request.source.nativeHandle() ==
                buffer_copy_request.destination.nativeHandle() &&
            source_range.overlaps(destination_range)) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "recordBufferCopy requires non-overlapping ranges when copying "
                "within the same buffer");
        }

        VkBufferCopy2 buffer_copy{};
        buffer_copy.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
        buffer_copy.size = buffer_copy_request.size;
        buffer_copy.srcOffset = buffer_copy_request.source_offset;
        buffer_copy.dstOffset = buffer_copy_request.destination_offset;

        VkCopyBufferInfo2 buffer_copy_info{};
        buffer_copy_info.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
        buffer_copy_info.srcBuffer = buffer_copy_request.source.nativeHandle();
        buffer_copy_info.dstBuffer =
            buffer_copy_request.destination.nativeHandle();
        buffer_copy_info.pRegions = &buffer_copy;
        buffer_copy_info.regionCount = 1U;

        vkCmdCopyBuffer2(command_buffer, &buffer_copy_info);
    }

    auto recordBufferToImageCopy(VkCommandBuffer command_buffer,
                                 const BufferToImageCopyRequest &request)
        -> void {
        if (command_buffer == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "recordBufferToImageCopy requires a valid command buffer");
        }

        if ((request.source.usage() & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) == 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "recordBufferToImageCopy requires the source buffer to support "
                "transfer-source usage");
        }

        if ((request.destination.usage() & VK_IMAGE_USAGE_TRANSFER_DST_BIT) ==
            0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "recordBufferToImageCopy requires the destination image to "
                "support transfer-destination usage");
        }

        if (request.regions.empty()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "recordBufferToImageCopy requires at least one copy region");
        }

        const std::optional<std::uint32_t> region_count =
            Core::Numeric::tryConvertToUint32(request.regions.size());

        if (!region_count.has_value()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "recordBufferToImageCopy region count exceeds uint32_t");
        }

        const VulkanImageCreateInfo &destination_info =
            request.destination.createInfo();

        for (const VkBufferImageCopy2 &region : request.regions) {
            validateBufferToImageRegion(region, destination_info);
            validateBufferToImageBounds(region, destination_info);
        }

        VkCopyBufferToImageInfo2 copy_info{};
        copy_info.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2;
        copy_info.srcBuffer = request.source.nativeHandle();
        copy_info.dstImage = request.destination.nativeHandle();
        copy_info.dstImageLayout = request.destination_layout;
        copy_info.regionCount = region_count.value();
        copy_info.pRegions = request.regions.data();

        vkCmdCopyBufferToImage2(command_buffer, &copy_info);
    }
} // namespace SNE::Engine::Renderer::Vulkan
