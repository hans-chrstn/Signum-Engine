#include "vulkan_command_pool.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/vulkan/vulkan_result.hpp"
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace SNE::Engine::Renderer::Vulkan {
    VulkanCommandPool::VulkanCommandPool(VkDevice device,
                                         std::uint32_t queue_family_index)
        : m_Device(device) {
        if (device == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanCommandPool requires a valid Vulkan device");
        }

        VkCommandPoolCreateInfo command_pool_create_info{};
        command_pool_create_info.sType =
            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.queueFamilyIndex = queue_family_index;
        command_pool_create_info.flags = 0;

        const VkResult command_pool_result = vkCreateCommandPool(
            m_Device, &command_pool_create_info, nullptr, &m_CommandPool);

        if (command_pool_result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanCommandPoolCreationFailed,
                "Failed to create Vulkan command pool",
                Core::Error::NativeError(
                    static_cast<int>(command_pool_result),
                    std::string(toString(command_pool_result))),
                "Create Vulkan Command Pool");
        }

        if (m_CommandPool == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Postcondition,
                Core::Error::Subsystem::Vulkan,
                "Successful Vulkan command-pool creation must produce a "
                "non-null command pool handle");
        }
    }

    VulkanCommandPool::~VulkanCommandPool() noexcept {
        destroy();
    }

    auto VulkanCommandPool::nativeHandle() const noexcept -> VkCommandPool {
        return m_CommandPool;
    }

    VulkanCommandPool::VulkanCommandPool(VulkanCommandPool &&other) noexcept
        : m_Device(std::exchange(other.m_Device, VK_NULL_HANDLE)),
          m_CommandPool(std::exchange(other.m_CommandPool, VK_NULL_HANDLE)) {}

    auto VulkanCommandPool::operator=(VulkanCommandPool &&other) noexcept
        -> VulkanCommandPool & {
        if (this == &other) {
            return *this;
        }

        destroy();

        m_Device = std::exchange(other.m_Device, VK_NULL_HANDLE);
        m_CommandPool = std::exchange(other.m_CommandPool, VK_NULL_HANDLE);

        return *this;
    }

    auto VulkanCommandPool::destroy() noexcept -> void {
        if (m_CommandPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
        }

        m_CommandPool = VK_NULL_HANDLE;
        m_Device = VK_NULL_HANDLE;
    }

    auto VulkanCommandPool::allocateCommandBuffers(VkCommandBufferLevel level,
                                                   std::uint32_t count)
        -> std::vector<VkCommandBuffer> {
        if (m_CommandPool == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanCommandPool must own a valid command pool before "
                "allocating command buffers");
        }

        if (count == 0) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "Command-buffer allocation count must be greater than zero");
        }

        std::vector<VkCommandBuffer> command_buffers(count);
        VkCommandBufferAllocateInfo allocate_info{};
        allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocate_info.commandPool = m_CommandPool;
        allocate_info.level = level;
        allocate_info.commandBufferCount = count;

        const VkResult result = vkAllocateCommandBuffers(
            m_Device, &allocate_info, command_buffers.data());
        if (result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanCommandBufferAllocationFailed,
                "Failed to allocate Vulkan command buffers",
                Core::Error::NativeError(static_cast<int>(result),
                                         std::string(toString(result))),
                "Allocate Vulkan Command Buffers");
        }

        for (const VkCommandBuffer &command_buffer : command_buffers) {
            if (command_buffer == VK_NULL_HANDLE) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Postcondition,
                    Core::Error::Subsystem::Vulkan,
                    "Successful Vulkan command-buffer allocation must produce "
                    "non-null command buffer handles");
            }
        }

        return command_buffers;
    }

    auto VulkanCommandPool::reset() -> void {
        if (m_CommandPool == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanCommandPool must own a valid command pool before "
                "resetting it");
        }

        const VkResult result = vkResetCommandPool(m_Device, m_CommandPool, 0U);

        if (result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanCommandPoolResetFailed,
                "Failed to reset Vulkan command pool",
                Core::Error::NativeError(static_cast<int>(result),
                                         std::string(toString(result))),
                "Reset Vulkan Command Pool");
        }
    }
} // namespace SNE::Engine::Renderer::Vulkan
