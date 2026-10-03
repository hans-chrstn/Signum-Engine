#include "vulkan_buffer.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/renderer/vulkan/vulkan_result.hpp"
#include <string>

namespace SNE::Engine::Renderer::Vulkan {
    VulkanBuffer::VulkanBuffer(VmaAllocator allocator,
                               const VulkanBufferCreateInfo &create_info)
        : m_Allocator(allocator) {
        VkBufferCreateInfo buffer_create_info{};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size = create_info.size;
        buffer_create_info.usage = create_info.usage;
        buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocation_create_info{};
        allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;
        allocation_create_info.requiredFlags =
            create_info.required_memory_properties;
        allocation_create_info.preferredFlags =
            create_info.preferred_memory_properties;

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

} // namespace SNE::Engine::Renderer::Vulkan
