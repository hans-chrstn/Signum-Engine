#include "vulkan_fence.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/vulkan/common/vulkan_result.hpp"
#include <cstdint>
#include <string>
#include <utility>

namespace SNE::Engine::Renderer::Vulkan {
    VulkanFence::VulkanFence(VkDevice device, bool initially_signaled)
        : m_Device(device) {
        if (device == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanFence requires a valid Vulkan device");
        }

        VkFenceCreateInfo fence_create_info{};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        if (initially_signaled) {
            fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        }

        const VkResult result =
            vkCreateFence(m_Device, &fence_create_info, nullptr, &m_Fence);

        if (result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanFenceCreationFailed,
                "Failed to create Vulkan fence",
                Core::Error::NativeError(static_cast<int>(result),
                                         std::string(toString(result))),
                "Create Vulkan Fence");
        }

        if (m_Fence == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Postcondition,
                Core::Error::Subsystem::Vulkan,
                "Successful Vulkan fence creation must produce a non-null "
                "fence handle");
        }
    }

    auto VulkanFence::destroy() noexcept -> void {
        if (m_Fence != VK_NULL_HANDLE) {
            vkDestroyFence(m_Device, m_Fence, nullptr);
        }

        m_Device = VK_NULL_HANDLE;
        m_Fence = VK_NULL_HANDLE;
    }

    VulkanFence::~VulkanFence() noexcept {
        destroy();
    }

    VulkanFence::VulkanFence(VulkanFence &&other) noexcept
        : m_Device(std::exchange(other.m_Device, VK_NULL_HANDLE)),
          m_Fence(std::exchange(other.m_Fence, VK_NULL_HANDLE)) {}

    auto VulkanFence::operator=(VulkanFence &&other) noexcept -> VulkanFence & {
        if (this == &other) {
            return *this;
        }

        destroy();

        m_Device = std::exchange(other.m_Device, VK_NULL_HANDLE);
        m_Fence = std::exchange(other.m_Fence, VK_NULL_HANDLE);

        return *this;
    }

    auto VulkanFence::nativeHandle() const noexcept -> VkFence {
        return m_Fence;
    }

    auto VulkanFence::wait() const -> void {
        if (m_Fence == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanFence must own a valid fence before waiting");
        }

        const VkResult result =
            vkWaitForFences(m_Device, 1U, &m_Fence, VK_TRUE, UINT64_MAX);
        if (result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanFenceWaitFailed,
                "Failed to wait for Vulkan fence",
                Core::Error::NativeError(static_cast<int>(result),
                                         std::string(toString(result))),
                "Wait for Vulkan Fence");
        }
    }

    auto VulkanFence::reset() -> void {
        if (m_Fence == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanFence must own a valid fence before resetting");
        }

        const VkResult result = vkResetFences(m_Device, 1U, &m_Fence);
        if (result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanFenceResetFailed,
                "Failed to reset Vulkan fence",
                Core::Error::NativeError(static_cast<int>(result),
                                         std::string(toString(result))),
                "Reset Vulkan Fence");
        }
    }

} // namespace SNE::Engine::Renderer::Vulkan
