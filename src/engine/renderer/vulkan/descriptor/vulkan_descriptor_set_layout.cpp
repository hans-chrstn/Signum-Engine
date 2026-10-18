#include "vulkan_descriptor_set_layout.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/shader/shader_descriptor_binding.hpp"
#include "engine/renderer/shader/shader_interface.hpp"
#include "engine/renderer/vulkan/common/vulkan_result.hpp"
#include "engine/renderer/vulkan/descriptor/vulkan_descriptor_type.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_shader_stage.hpp"
#include <cstddef>
#include <map>
#include <string>
#include <utility>

namespace SNE::Engine::Renderer::Vulkan {
    auto makeVkDescriptorSetLayoutBinding(
        const Shader::ShaderDescriptorBinding &requirement,
        Shader::ShaderStage stage) -> VkDescriptorSetLayoutBinding {
        if (requirement.count == 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "Vulkan descriptor-set layout binding requires a nonzero "
                "descriptor count");
        }

        return VkDescriptorSetLayoutBinding{
            .binding = requirement.binding,
            .descriptorType = toVkDescriptorType(requirement.type),
            .descriptorCount = requirement.count,
            .stageFlags = toVkShaderStage(stage),
            .pImmutableSamplers = nullptr,
        };
    }

    auto mergeVkDescriptorSetLayoutRequirements(
        std::span<const Shader::ShaderInterface> interfaces)
        -> std::vector<VulkanDescriptorSetLayoutRequirements> {
        std::map<std::uint32_t,
                 std::map<std::uint32_t, VkDescriptorSetLayoutBinding>>
            grouped_bindings;

        for (const Shader::ShaderInterface &interface : interfaces) {
            for (const Shader::ShaderDescriptorBinding &requirement :
                 interface.descriptor_bindings) {
                const VkDescriptorSetLayoutBinding binding =
                    makeVkDescriptorSetLayoutBinding(
                        requirement, interface.entry_point.stage);

                auto &bindings = grouped_bindings[requirement.set];

                auto [binding_it, inserted] =
                    bindings.try_emplace(requirement.binding, binding);

                if (inserted) {
                    continue;
                }

                VkDescriptorSetLayoutBinding &existing = binding_it->second;

                if (existing.descriptorType != binding.descriptorType ||
                    existing.descriptorCount != binding.descriptorCount) {
                    throw Core::Error::EngineError(
                        Core::Error::Code::VulkanDescriptorLayoutIncompatible,
                        "Shader descriptor requirements have incompatible "
                        "types or counts at the same descriptor location",
                        "Merge Vulkan Descriptor Set Layout Requirements");
                }

                existing.stageFlags |= binding.stageFlags;
            }
        }

        std::vector<VulkanDescriptorSetLayoutRequirements> groups;
        groups.reserve(grouped_bindings.size());

        for (const auto &[set, bindings] : grouped_bindings) {
            VulkanDescriptorSetLayoutRequirements group{
                .set = set,
                .bindings = {},
            };

            group.bindings.reserve(bindings.size());

            for (const auto &entry : bindings) {
                group.bindings.push_back(entry.second);
            }

            groups.push_back(std::move(group));
        }

        return groups;
    }

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

        m_DescriptorBindingDescriptions.reserve(create_info.bindingCount);

        for (std::size_t i{}; i < create_info.bindingCount; ++i) {
            const auto &bindings = create_info.pBindings[i];
            VulkanDescriptorBindingDescription binding_description{
                .binding = bindings.binding,
                .descriptor_type = bindings.descriptorType,
                .descriptor_count = bindings.descriptorCount,
                .stage_flags = bindings.stageFlags,
            };

            m_DescriptorBindingDescriptions.push_back(binding_description);
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
        m_DescriptorBindingDescriptions.clear();
    }

    VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() noexcept {
        destroy();
    }

    VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(
        VulkanDescriptorSetLayout &&other) noexcept
        : m_Device(std::exchange(other.m_Device, VK_NULL_HANDLE)),
          m_DescriptorSetLayout(
              std::exchange(other.m_DescriptorSetLayout, VK_NULL_HANDLE)),
          m_DescriptorBindingDescriptions(
              std::move(other.m_DescriptorBindingDescriptions)) {}

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
        m_DescriptorBindingDescriptions =
            std::move(other.m_DescriptorBindingDescriptions);

        return *this;
    }

    auto VulkanDescriptorSetLayout::nativeHandle() const noexcept
        -> VkDescriptorSetLayout {
        return m_DescriptorSetLayout;
    }

    auto VulkanDescriptorSetLayout::bindingDescriptions() const noexcept
        -> std::span<const VulkanDescriptorBindingDescription> {
        return m_DescriptorBindingDescriptions;
    }
} // namespace SNE::Engine::Renderer::Vulkan
