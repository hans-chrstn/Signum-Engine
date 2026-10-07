#include "vulkan_transfer_commands.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/vulkan/memory/vulkan_byte_range.hpp"

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
} // namespace SNE::Engine::Renderer::Vulkan
