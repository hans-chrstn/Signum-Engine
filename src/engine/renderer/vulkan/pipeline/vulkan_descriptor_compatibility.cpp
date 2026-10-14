#include "vulkan_descriptor_compatibility.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/shader/shader_descriptor_binding.hpp"
#include "engine/renderer/shader/shader_resource_type.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include <cstddef>
#include <span>
#include <utility>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;
namespace Core = SNE::Engine::Core;
namespace Shader = SNE::Engine::Renderer::Shader;

namespace {
    [[nodiscard]] auto toVkDescriptorType(Shader::ShaderResourceType type)
        -> VkDescriptorType {
        switch (type) {
        case Shader::ShaderResourceType::CombinedImageSampler:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case Shader::ShaderResourceType::UniformBuffer:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case Shader::ShaderResourceType::StorageBuffer:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        default:
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "Descriptor compatibility validation requires a supported "
                "shader resource type");
        }
    }

    [[nodiscard]] auto toVkShaderStage(Shader::ShaderStage stage)
        -> VkShaderStageFlagBits {
        switch (stage) {
        case Shader::ShaderStage::Vertex:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case Shader::ShaderStage::Fragment:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        case Shader::ShaderStage::Compute:
            return VK_SHADER_STAGE_COMPUTE_BIT;
        default:
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "Descriptor compatibility validation requires a supported "
                "shader stage");
        }
    }

    auto validatePipelineLayoutDescriptions(
        std::span<const Vulkan::DescriptorSetLayoutBindingView>
            pipeline_layouts) -> void {
        for (const Vulkan::DescriptorSetLayoutBindingView &layout :
             pipeline_layouts) {
            for (std::size_t i{}; i < layout.bindings.size(); ++i) {
                const Vulkan::VulkanDescriptorBindingDescription &current =
                    layout.bindings[i];

                if (current.descriptor_count == 0U) {
                    Core::Assertion::failAssertion(
                        Core::Assertion::AssertionType::Precondition,
                        Core::Error::Subsystem::Vulkan,
                        "Pipeline descriptor binding must have a nonzero "
                        "descriptor count");
                }

                for (std::size_t j = i + 1U; j < layout.bindings.size(); ++j) {
                    if (current.binding == layout.bindings[j].binding) {
                        Core::Assertion::failAssertion(
                            Core::Assertion::AssertionType::Precondition,
                            Core::Error::Subsystem::Vulkan,
                            "Pipeline descriptor-set layout contains duplicate "
                            "binding indices");
                    }
                }
            }
        }
    }

    auto validateMatchingDescriptorBinding(
        const Shader::ShaderDescriptorBinding &required,
        const Vulkan::VulkanDescriptorBindingDescription &provided,
        Shader::ShaderStage shader_stage) -> void {
        if (toVkDescriptorType(required.type) != provided.descriptor_type) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanDescriptorLayoutIncompatible,
                "Pipeline descriptor binding type does not match shader "
                "requirement",
                "Validate Descriptor Compatibility");
        }

        if (provided.descriptor_count < required.count) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanDescriptorLayoutIncompatible,
                "Pipeline descriptor binding count is smaller than shader "
                "requirement",
                "Validate Descriptor Compatibility");
        }

        const auto required_stage =
            static_cast<VkShaderStageFlags>(toVkShaderStage(shader_stage));

        if ((provided.stage_flags & required_stage) == 0U) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanDescriptorLayoutIncompatible,
                "Pipeline descriptor binding does not include the required "
                "shader stage",
                "Validate Descriptor Compatibility");
        }
    }

    auto validateShaderDescriptorRequirement(
        const Shader::ShaderDescriptorBinding &required,
        Shader::ShaderStage shader_stage,
        std::span<const Vulkan::DescriptorSetLayoutBindingView>
            pipeline_layouts) -> void {
        if (required.count == 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "Shader descriptor requirement must have a nonzero "
                "descriptor count");
        }

        if (std::cmp_greater_equal(required.set, pipeline_layouts.size())) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanDescriptorLayoutIncompatible,
                "Pipeline layout does not contain the required shader "
                "descriptor set",
                "Validate Descriptor Compatibility");
        }

        const auto &bindings = pipeline_layouts[required.set].bindings;

        for (const Vulkan::VulkanDescriptorBindingDescription &provided :
             bindings) {
            if (provided.binding != required.binding) {
                continue;
            }

            validateMatchingDescriptorBinding(required, provided, shader_stage);
            return;
        }

        throw Core::Error::EngineError(
            Core::Error::Code::VulkanDescriptorLayoutIncompatible,
            "Pipeline descriptor set does not contain the required "
            "shader binding",
            "Validate Descriptor Compatibility");
    }
} // namespace

namespace SNE::Engine::Renderer::Vulkan {
    auto validateDescriptorCompatibility(
        const Shader::ShaderInterface &shader_interface,
        std::span<const DescriptorSetLayoutBindingView> pipeline_layouts)
        -> void {
        validatePipelineLayoutDescriptions(pipeline_layouts);

        for (const Shader::ShaderDescriptorBinding &required :
             shader_interface.descriptor_bindings) {
            validateShaderDescriptorRequirement(
                required, shader_interface.entry_point.stage, pipeline_layouts);
        }
    }
} // namespace SNE::Engine::Renderer::Vulkan
