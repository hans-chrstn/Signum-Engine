#include "vulkan_descriptor_pool.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/core/numeric/checked_conversion.hpp"
#include "engine/renderer/vulkan/common/vulkan_result.hpp"
#include <string>
#include <utility>
#include <vector>

namespace SNE::Engine::Renderer::Vulkan {
    VulkanDescriptorPool::VulkanDescriptorPool(
        VkDevice device, const VkDescriptorPoolCreateInfo &create_info)
        : m_Device(device) {
        if (device == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanDescriptorPool requires a valid Vulkan device");
        }

        if (create_info.sType !=
            VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanDescriptorPool requires a valid descriptor-pool "
                "creation structure type");
        }

        if (create_info.maxSets == 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanDescriptorPool requires at least one allocatable "
                "descriptor set");
        }

        if (create_info.poolSizeCount > 0U &&
            create_info.pPoolSizes == nullptr) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanDescriptorPool requires pool-size descriptions when "
                "pool size count is non-zero");
        }

        const VkResult result = vkCreateDescriptorPool(
            m_Device, &create_info, nullptr, &m_DescriptorPool);
        if (result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanDescriptorPoolCreationFailed,
                "Failed to create Vulkan descriptor pool",
                Core::Error::NativeError(static_cast<int>(result),
                                         std::string(toString(result))),
                "Create Vulkan Descriptor Pool");
        }

        if (m_DescriptorPool == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Postcondition,
                Core::Error::Subsystem::Vulkan,
                "Successful Vulkan descriptor-pool creation must produce "
                "a non-null pool handle");
        }
    }

    auto VulkanDescriptorPool::destroy() noexcept -> void {
        if (m_DescriptorPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
        }

        m_Device = VK_NULL_HANDLE;
        m_DescriptorPool = VK_NULL_HANDLE;
    }

    VulkanDescriptorPool::~VulkanDescriptorPool() noexcept {
        destroy();
    }

    VulkanDescriptorPool::VulkanDescriptorPool(
        VulkanDescriptorPool &&other) noexcept
        : m_Device(std::exchange(other.m_Device, VK_NULL_HANDLE)),
          m_DescriptorPool(
              std::exchange(other.m_DescriptorPool, VK_NULL_HANDLE)) {}

    auto VulkanDescriptorPool::operator=(VulkanDescriptorPool &&other) noexcept
        -> VulkanDescriptorPool & {
        if (this == &other) {
            return *this;
        }

        destroy();

        m_Device = std::exchange(other.m_Device, VK_NULL_HANDLE);
        m_DescriptorPool =
            std::exchange(other.m_DescriptorPool, VK_NULL_HANDLE);

        return *this;
    }

    auto VulkanDescriptorPool::nativeHandle() const noexcept
        -> VkDescriptorPool {
        return m_DescriptorPool;
    }

    auto VulkanDescriptorPool::allocateDescriptorSets(
        std::span<const VkDescriptorSetLayout> layouts)
        -> std::vector<VkDescriptorSet> {
        if (m_DescriptorPool == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanDescriptorPool requires a valid descriptor pool for "
                "allocation");
        }

        if (layouts.empty()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanDescriptorPool requires at least one descriptor-set "
                "layout");
        }

        for (VkDescriptorSetLayout layout : layouts) {
            if (layout == VK_NULL_HANDLE) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanDescriptorPool requires valid descriptor-set layout "
                    "handles");
            }
        }

        const auto descriptor_set_count =
            Core::Numeric::tryConvertToUint32(layouts.size());

        if (!descriptor_set_count.has_value()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanDescriptorPool descriptor-set allocation count exceeds "
                "uint32_t");
        }

        VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
        descriptor_set_allocate_info.sType =
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptor_set_allocate_info.descriptorPool = m_DescriptorPool;
        descriptor_set_allocate_info.descriptorSetCount =
            descriptor_set_count.value();
        descriptor_set_allocate_info.pSetLayouts = layouts.data();

        std::vector<VkDescriptorSet> descriptor_sets{};

        descriptor_sets.resize(layouts.size());

        const VkResult result = vkAllocateDescriptorSets(
            m_Device, &descriptor_set_allocate_info, descriptor_sets.data());

        if (result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanDescriptorSetAllocationFailed,
                "Failed to allocate Vulkan descriptor sets",
                Core::Error::NativeError(static_cast<int>(result),
                                         std::string(toString(result))),
                "Allocate Vulkan Descriptor Sets");
        }

        for (VkDescriptorSet descriptor_set : descriptor_sets) {
            if (descriptor_set == VK_NULL_HANDLE) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Postcondition,
                    Core::Error::Subsystem::Vulkan,
                    "Successful Vulkan descriptor-set allocation must produce "
                    "non-null descriptor-set handles");
            }
        }

        return descriptor_sets;
    }
} // namespace SNE::Engine::Renderer::Vulkan
