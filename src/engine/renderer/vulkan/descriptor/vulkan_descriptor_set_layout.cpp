#include "vulkan_descriptor_set_layout.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/vulkan/common/vulkan_result.hpp"
#include <string>
#include <utility>

namespace SNE::Engine::Renderer::Vulkan {
    VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(
        VkDevice device, const VkDescriptorSetLayoutCreateInfo &create_info)
        : m_Device(device) {
        if (device == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanDescriptorSetLayout requires a valid Vulkan device");
        }

        if (create_info.sType !=
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanDescriptorSetLayout requires a valid descriptor-set "
                "layout creation structure type");
        }

        if (create_info.pBindings == nullptr && create_info.bindingCount > 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanDescriptorSetLayout requires binding descriptions when "
                "binding count is non-zero");
        }

        const VkResult result = vkCreateDescriptorSetLayout(
            m_Device, &create_info, nullptr, &m_DescriptorSetLayout);
        if (result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanDescriptorSetLayoutCreationFailed,
                "Failed to create Vulkan descriptor-set layout",
                Core::Error::NativeError(static_cast<int>(result),
                                         std::string(toString(result))),
                "Create Vulkan Descriptor Set Layout");
        }

        if (m_DescriptorSetLayout == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Postcondition,
                Core::Error::Subsystem::Vulkan,
                "Successful Vulkan descriptor-set layout creation must produce "
                "a non-null layout handle");
        }
    }

    auto VulkanDescriptorSetLayout::destroy() noexcept -> void {
        if (m_DescriptorSetLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout,
                                         nullptr);
        }

        m_Device = VK_NULL_HANDLE;
        m_DescriptorSetLayout = VK_NULL_HANDLE;
    }

    VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() noexcept {
        destroy();
    }

    VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(
        VulkanDescriptorSetLayout &&other) noexcept
        : m_Device(std::exchange(other.m_Device, VK_NULL_HANDLE)),
          m_DescriptorSetLayout(
              std::exchange(other.m_DescriptorSetLayout, VK_NULL_HANDLE)) {}

    auto VulkanDescriptorSetLayout::operator=(
        VulkanDescriptorSetLayout &&other) noexcept
        -> VulkanDescriptorSetLayout & {
        if (this == &other) {
            return *this;
        }

        destroy();

        m_Device = std::exchange(other.m_Device, VK_NULL_HANDLE);
        m_DescriptorSetLayout =
            std::exchange(other.m_DescriptorSetLayout, VK_NULL_HANDLE);

        return *this;
    }

    auto VulkanDescriptorSetLayout::nativeHandle() const noexcept
        -> VkDescriptorSetLayout {
        return m_DescriptorSetLayout;
    }
} // namespace SNE::Engine::Renderer::Vulkan
