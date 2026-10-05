#include "vulkan_upload.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/gpu_memory_usage.hpp"
#include "vulkan_transfer_commands.hpp"

namespace SNE::Engine::Renderer::Vulkan {
    auto uploadBufferData(const VulkanMemoryAllocator &allocator,
                          VulkanImmediateSubmission &immediate_submission,
                          const VulkanBuffer &destination,
                          std::span<const std::byte> bytes,
                          VkDeviceSize destination_offset) -> void {
        if (allocator.nativeHandle() == nullptr) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "uploadBufferData requires a valid VMA allocator");
        }

        if (bytes.empty()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "uploadBufferData requires non-empty data");
        }

        const auto bytes_size = static_cast<VkDeviceSize>(bytes.size());

        if (destination_offset > destination.size() ||
            bytes_size > destination.size() - destination_offset) {
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
} // namespace SNE::Engine::Renderer::Vulkan
