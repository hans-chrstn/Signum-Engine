#include "vulkan_readback.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/gpu_memory_usage.hpp"
#include "engine/renderer/vulkan/command/vulkan_transfer_commands.hpp"
#include "vulkan_buffer.hpp"
#include "vulkan_byte_range.hpp"

namespace SNE::Engine::Renderer::Vulkan {
    auto readBufferData(const VulkanMemoryAllocator &allocator,
                        VulkanImmediateSubmission &immediate_submission,
                        const VulkanBuffer &source, std::span<std::byte> output,
                        VkDeviceSize source_offset) -> void {
        if (output.empty()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "readBufferData requires a non-empty output span");
        }

        const auto output_size = static_cast<VkDeviceSize>(output.size());

        const ByteRange read_range{
            .offset = source_offset,
            .size = output_size,
        };

        if (!read_range.fitsWithin(source.size())) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "readBufferData range exceeds the source buffer size");
        }

        VulkanBufferCreateInfo buffer_create_info{};
        buffer_create_info.size = output_size;
        buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        buffer_create_info.memory_usage = GpuMemoryUsage::Readback;

        VulkanBuffer temporary_readback_buffer =
            VulkanBuffer(allocator, buffer_create_info);

        immediate_submission.execute(
            [&](VkCommandBuffer command_buffer) -> void {
                BufferCopyRequest copy_request{
                    .source = source,
                    .destination = temporary_readback_buffer,
                    .source_offset = source_offset,
                    .destination_offset = 0U,
                    .size = output_size,
                };
                recordBufferCopy(command_buffer, copy_request);
            });

        temporary_readback_buffer.read(output);
    }

} // namespace SNE::Engine::Renderer::Vulkan
