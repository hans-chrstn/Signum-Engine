#include "vulkan_buffer.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/gpu_memory_usage.hpp"
#include "engine/renderer/vulkan/vulkan_result.hpp"
#include "vulkan_memory_policy.hpp"
#include <cstddef>
#include <cstring>
#include <string>

namespace SNE::Engine::Renderer::Vulkan {
    VulkanBuffer::VulkanBuffer(VmaAllocator allocator,
                               const VulkanBufferCreateInfo &create_info)
        : m_Allocator(allocator), m_Size(create_info.size),
          m_MemoryUsage(create_info.memory_usage) {
        if (allocator == nullptr) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanBuffer requires a valid VMA allocator");
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

        if (offset > m_Size ||
            static_cast<VkDeviceSize>(bytes.size()) > m_Size - offset) {
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
        std::memcpy(mapped_byte, bytes.data(), bytes.size());

        // make cpu writes visible to the gpu for non coherent memory
        const VkResult flush_result =
            vmaFlushAllocation(m_Allocator, m_Allocation, offset,
                               static_cast<VkDeviceSize>(bytes.size()));

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
} // namespace SNE::Engine::Renderer::Vulkan
