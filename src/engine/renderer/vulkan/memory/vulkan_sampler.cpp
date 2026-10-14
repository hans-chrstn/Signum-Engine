#include "vulkan_sampler.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/renderer/vulkan/common/vulkan_result.hpp"
#include <string>
#include <utility>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    VulkanSampler::VulkanSampler(VkDevice device,
                                 const VkSamplerCreateInfo &create_info)
        : m_Device(device) {
        if (device == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanSampler requires a valid Vulkan device");
        }

        if (create_info.sType != VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanSampler requires a valid sampler creation structure "
                "type");
        }

        const VkResult result =
            vkCreateSampler(m_Device, &create_info, nullptr, &m_Sampler);
        if (result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanSamplerCreationFailed,
                "Failed to create Vulkan sampler",
                Core::Error::NativeError(static_cast<int>(result),
                                         std::string(toString(result))),
                "Create Vulkan Sampler");
        }

        if (m_Sampler == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Postcondition,
                Core::Error::Subsystem::Vulkan,
                "Successful Vulkan sampler creation must produce a non-null "
                "sampler handle");
        }
    }

    auto VulkanSampler::destroy() noexcept -> void {
        if (m_Sampler != VK_NULL_HANDLE) {
            vkDestroySampler(m_Device, m_Sampler, nullptr);
        }

        m_Sampler = VK_NULL_HANDLE;
        m_Device = VK_NULL_HANDLE;
    }

    VulkanSampler::~VulkanSampler() noexcept {
        destroy();
    }

    VulkanSampler::VulkanSampler(VulkanSampler &&other) noexcept
        : m_Device(std::exchange(other.m_Device, VK_NULL_HANDLE)),
          m_Sampler(std::exchange(other.m_Sampler, VK_NULL_HANDLE)) {}

    auto VulkanSampler::operator=(VulkanSampler &&other) noexcept
        -> VulkanSampler & {
        if (this == &other) {
            return *this;
        }

        destroy();

        m_Device = std::exchange(other.m_Device, VK_NULL_HANDLE);
        m_Sampler = std::exchange(other.m_Sampler, VK_NULL_HANDLE);

        return *this;
    }

    auto VulkanSampler::nativeHandle() const noexcept -> VkSampler {
        return m_Sampler;
    }
} // namespace SNE::Engine::Renderer::Vulkan
