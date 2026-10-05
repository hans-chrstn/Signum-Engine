#include "vulkan_buffer.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/gpu_memory_usage.hpp"
#include "vulkan_memory_policy.hpp"
#include "vulkan_result.hpp"
#include <cstddef>
#include <cstring>
#include <string>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;
namespace Core = SNE::Engine::Core;

namespace SNE::Engine::Renderer::Vulkan {
    VulkanBuffer::VulkanBuffer(const VulkanMemoryAllocator &allocator,
                               const VulkanBufferCreateInfo &create_info)
        : m_Allocator(allocator.nativeHandle()), m_Size(create_info.size),
          m_MemoryUsage(create_info.memory_usage) {
        if (allocator.nativeHandle() == nullptr) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanBuffer requires a valid Vulkan memory allocator");
        }

        if (create_info.size == 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanBuffer requires a non-zero buffer size");
        }

        if (create_info.usage == 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanBuffer requires at least one buffer usage flag");
        }

        VkBufferCreateInfo buffer_create_info{};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size = create_info.size;
        buffer_create_info.usage = create_info.usage;
        buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        const VulkanMemoryPolicy memory_policy =
            selectVulkanMemoryPolicy(create_info.memory_usage);

        VmaAllocationCreateInfo allocation_create_info{};
        allocation_create_info.usage = memory_policy.usage;
        allocation_create_info.flags = memory_policy.flags;

        const VkResult buffer_result = vmaCreateBuffer(
            m_Allocator, &buffer_create_info, &allocation_create_info,
            &m_Buffer, &m_Allocation, nullptr);

        if (buffer_result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanBufferCreationFailed,
                "Failed to create Vulkan buffer",
                Core::Error::NativeError(static_cast<int>(buffer_result),
                                         std::string(toString(buffer_result))),
                "Create Vulkan Buffer");
        }
    }

    VulkanBuffer::~VulkanBuffer() noexcept {
        if (m_Buffer != VK_NULL_HANDLE && m_Allocation != nullptr) {
            vmaDestroyBuffer(m_Allocator, m_Buffer, m_Allocation);
        }

        m_Buffer = VK_NULL_HANDLE;
        m_Allocation = nullptr;
        m_Allocator = nullptr;
    }

    auto VulkanBuffer::write(std::span<const std::byte> bytes,
                             VkDeviceSize offset) -> void {
        if (m_MemoryUsage != GpuMemoryUsage::Upload) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanBuffer::write requires an upload buffer");
        }

        if (bytes.empty()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanBuffer::write requires non-empty data");
        }

        const auto bytes_size = static_cast<VkDeviceSize>(bytes.size());

        if (offset > m_Size || bytes_size > m_Size - offset) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanBuffer::write range exceeds the buffer size");
        }

        void *mapped_data = nullptr;

        // map the vma allocation so the cpu can access its memory
        const VkResult map_result =
            vmaMapMemory(m_Allocator, m_Allocation, &mapped_data);

        if (map_result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanBufferMappingFailed,
                "Failed to map Vulkan buffer allocation",
                Core::Error::NativeError(static_cast<int>(map_result),
                                         std::string(toString(map_result))),
                "Map Vulkan Buffer Allocation");
        }

        // treat the mapped address as raw bytes so byte offsets can be applied
        auto *mapped_byte = static_cast<std::byte *>(mapped_data) + offset;

        // copy the caller's bytes into the mapped allocation
        std::memcpy(mapped_byte, bytes.data(), bytes_size);

        // make cpu writes visible to the gpu for non coherent memory
        const VkResult flush_result =
            vmaFlushAllocation(m_Allocator, m_Allocation, offset, bytes_size);

        // temporary cpu mapping is not needed anymore since we already wrote it
        vmaUnmapMemory(m_Allocator, m_Allocation);

        if (flush_result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanBufferFlushFailed,
                "Failed to flush Vulkan buffer allocation",
                Core::Error::NativeError(static_cast<int>(flush_result),
                                         std::string(toString(flush_result))),
                "Flush Vulkan Buffer Allocation");
        }
    }

    auto VulkanBuffer::nativeHandle() const noexcept -> VkBuffer {
        return m_Buffer;
    }

    auto VulkanBuffer::size() const noexcept -> VkDeviceSize {
        return m_Size;
    }

    auto VulkanBuffer::read(std::span<std::byte> output, VkDeviceSize offset)
        -> void {
        if (m_MemoryUsage != GpuMemoryUsage::Readback) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanBuffer::read requires GpuMemoryUsage::Readback");
        }

        if (output.empty()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanBuffer::read requires a non-empty output span");
        }

        const auto output_size = static_cast<VkDeviceSize>(output.size());

        if (offset > m_Size || output_size > m_Size - offset) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanBuffer::read range exceeds the buffer size");
        }

        void *mapped_data = nullptr;
        const VkResult map_result =
            vmaMapMemory(m_Allocator, m_Allocation, &mapped_data);

        if (map_result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanBufferMappingFailed,
                "Failed to map Vulkan buffer allocation",
                Core::Error::NativeError(static_cast<int>(map_result),
                                         std::string(toString(map_result))),
                "Map Vulkan Buffer Allocation");
        }

        const VkResult invalidate_result = vmaInvalidateAllocation(
            m_Allocator, m_Allocation, offset, output_size);

        if (invalidate_result != VK_SUCCESS) {
            vmaUnmapMemory(m_Allocator, m_Allocation);
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanBufferInvalidationFailed,
                "Failed to invalidate Vulkan buffer allocation",
                Core::Error::NativeError(
                    static_cast<int>(invalidate_result),
                    std::string(toString(invalidate_result))),
                "Invalidate Vulkan Buffer Allocation");
        }

        auto *mapped_byte = static_cast<std::byte *>(mapped_data) + offset;

        std::memcpy(output.data(), mapped_byte, output_size);
        vmaUnmapMemory(m_Allocator, m_Allocation);
    }
} // namespace SNE::Engine::Renderer::Vulkan
