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
} // namespace SNE::Engine::Renderer::Vulkan
