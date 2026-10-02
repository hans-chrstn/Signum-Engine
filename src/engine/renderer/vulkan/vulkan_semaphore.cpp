#include "vulkan_semaphore.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/vulkan/vulkan_result.hpp"
#include <string>
#include <utility>

namespace SNE::Engine::Renderer::Vulkan {
    VulkanSemaphore::VulkanSemaphore(VkDevice device) : m_Device(device) {
        if (device == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanSemaphore requires a valid Vulkan device");
        }

        VkSemaphoreCreateInfo semaphore_create_info{};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        const VkResult result = vkCreateSemaphore(
            m_Device, &semaphore_create_info, nullptr, &m_Semaphore);
        if (result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanSemaphoreCreationFailed,
                "Failed to create Vulkan semaphore",
                Core::Error::NativeError(static_cast<int>(result),
                                         std::string(toString(result))),
                "Create Vulkan Semaphore");
        }
    }

    auto VulkanSemaphore::destroy() noexcept -> void {
        if (m_Semaphore != VK_NULL_HANDLE) {
            vkDestroySemaphore(m_Device, m_Semaphore, nullptr);
        }
        m_Device = VK_NULL_HANDLE;
        m_Semaphore = VK_NULL_HANDLE;
    }

    VulkanSemaphore::~VulkanSemaphore() noexcept {
        destroy();
    }

    VulkanSemaphore::VulkanSemaphore(VulkanSemaphore &&other) noexcept
        : m_Device(std::exchange(other.m_Device, VK_NULL_HANDLE)),
          m_Semaphore(std::exchange(other.m_Semaphore, VK_NULL_HANDLE)) {}

    auto VulkanSemaphore::operator=(VulkanSemaphore &&other) noexcept
        -> VulkanSemaphore & {
        if (this == &other) {
            return *this;
        }

        destroy();

        m_Device = std::exchange(other.m_Device, VK_NULL_HANDLE);
        m_Semaphore = std::exchange(other.m_Semaphore, VK_NULL_HANDLE);

        return *this;
    }

    auto VulkanSemaphore::nativeHandle() const noexcept -> VkSemaphore {
        return m_Semaphore;
    }
} // namespace SNE::Engine::Renderer::Vulkan
