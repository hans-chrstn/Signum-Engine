#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/renderer/shader/shader_descriptor_binding.hpp"
#include "engine/renderer/shader/shader_interface.hpp"
#include "engine/renderer/shader/shader_resource_type.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_descriptor_compatibility.hpp"
#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <initializer_list>
#include <span>
#include <vulkan/vulkan.h>

namespace Core = SNE::Engine::Core;
namespace Shader = SNE::Engine::Renderer::Shader;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    constexpr std::uint32_t kSetZero = 0U;
    constexpr std::uint32_t kSetOne = 1U;
    constexpr std::uint32_t kBindingZero = 0U;
    constexpr std::uint32_t kBindingOne = 1U;
    constexpr std::uint32_t kBindingTwo = 2U;
    constexpr std::uint32_t kOneDescriptor = 1U;
    constexpr std::uint32_t kTwoDescriptors = 2U;
    constexpr std::uint32_t kFourDescriptors = 4U;

    constexpr VkShaderStageFlags kVertexStage =
        static_cast<VkShaderStageFlags>(VK_SHADER_STAGE_VERTEX_BIT);
    constexpr VkShaderStageFlags kFragmentStage =
        static_cast<VkShaderStageFlags>(VK_SHADER_STAGE_FRAGMENT_BIT);
    constexpr VkShaderStageFlags kComputeStage =
        static_cast<VkShaderStageFlags>(VK_SHADER_STAGE_COMPUTE_BIT);
    constexpr VkShaderStageFlags kVertexFragmentStages =
        kVertexStage | kFragmentStage;

    [[nodiscard]] constexpr auto
    requiredBinding(std::uint32_t set, std::uint32_t binding,
                    Shader::ShaderResourceType type,
                    std::uint32_t count = kOneDescriptor)
        -> Shader::ShaderDescriptorBinding {
        return {
            .set = set,
            .binding = binding,
            .type = type,
            .count = count,
        };
    }

    [[nodiscard]] constexpr auto
    providedBinding(std::uint32_t binding, VkDescriptorType type,
                    VkShaderStageFlags stages,
                    std::uint32_t count = kOneDescriptor)
        -> Vulkan::VulkanDescriptorBindingDescription {
        return {
            .binding = binding,
            .descriptor_type = type,
            .descriptor_count = count,
            .stage_flags = stages,
        };
    }

    [[nodiscard]] auto makeInterface(
        Shader::ShaderStage stage,
        std::initializer_list<Shader::ShaderDescriptorBinding> requirements)
        -> Shader::ShaderInterface {
        Shader::ShaderInterface result{};
        result.entry_point.stage = stage;
        result.entry_point.entry_point = "main";
        result.descriptor_bindings.assign(requirements);
        return result;
    }

    [[nodiscard]] auto layoutView(
        std::span<const Vulkan::VulkanDescriptorBindingDescription> bindings)
        -> Vulkan::DescriptorSetLayoutBindingView {
        return {
            .bindings = bindings,
        };
    }

    auto expectIncompatible(
        const Shader::ShaderInterface &shader_interface,
        std::span<const Vulkan::DescriptorSetLayoutBindingView> layouts,
        const char *expected_message) -> void {
        try {
            Vulkan::validateDescriptorCompatibility(shader_interface, layouts);
            FAIL() << "Expected incompatible descriptor layout";
        } catch (const Core::Error::EngineError &error) {
            EXPECT_EQ(error.getCode(),
                      Core::Error::Code::VulkanDescriptorLayoutIncompatible);
            EXPECT_STREQ(error.what(), expected_message);
            EXPECT_EQ(error.getOperation(),
                      "Validate Descriptor Compatibility");
        }
    }
} // namespace

TEST(DescriptorCompatibilityTests, AcceptsEmptyShaderAndPipelineLayouts) {
    const Shader::ShaderInterface shader_interface{};

    EXPECT_NO_THROW(
        Vulkan::validateDescriptorCompatibility(shader_interface, {}));
}

TEST(DescriptorCompatibilityTests, AcceptsUnusedPipelineBinding) {
    const Shader::ShaderInterface shader_interface{};

    const std::array bindings{
        providedBinding(kBindingZero, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        kVertexStage),
    };
    const std::array layouts{layoutView(bindings)};

    EXPECT_NO_THROW(
        Vulkan::validateDescriptorCompatibility(shader_interface, layouts));
}

TEST(DescriptorCompatibilityTests, AcceptsEmptyDescriptorSetLayout) {
    const Shader::ShaderInterface shader_interface{};
    const std::array layouts{layoutView({})};

    EXPECT_NO_THROW(
        Vulkan::validateDescriptorCompatibility(shader_interface, layouts));
}

TEST(DescriptorCompatibilityTests, AcceptsExactCombinedImageSampler) {
    const auto shader_interface = makeInterface(
        Shader::ShaderStage::Fragment,
        {
            requiredBinding(kSetZero, kBindingZero,
                            Shader::ShaderResourceType::CombinedImageSampler),
        });

    const std::array bindings{
        providedBinding(kBindingZero, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        kFragmentStage),
    };
    const std::array layouts{layoutView(bindings)};

    EXPECT_NO_THROW(
        Vulkan::validateDescriptorCompatibility(shader_interface, layouts));
}

TEST(DescriptorCompatibilityTests, AcceptsUniformAndStorageBuffers) {
    const auto shader_interface = makeInterface(
        Shader::ShaderStage::Compute,
        {
            requiredBinding(kSetZero, kBindingZero,
                            Shader::ShaderResourceType::UniformBuffer),
            requiredBinding(kSetZero, kBindingOne,
                            Shader::ShaderResourceType::StorageBuffer),
        });

    const std::array bindings{
        providedBinding(kBindingZero, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        kComputeStage),
        providedBinding(kBindingOne, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        kComputeStage),
    };
    const std::array layouts{layoutView(bindings)};

    EXPECT_NO_THROW(
        Vulkan::validateDescriptorCompatibility(shader_interface, layouts));
}

TEST(DescriptorCompatibilityTests, AcceptsLargerDescriptorArray) {
    const auto shader_interface = makeInterface(
        Shader::ShaderStage::Fragment,
        {
            requiredBinding(kSetZero, kBindingZero,
                            Shader::ShaderResourceType::CombinedImageSampler,
                            kTwoDescriptors),
        });

    const std::array bindings{
        providedBinding(kBindingZero, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        kFragmentStage, kFourDescriptors),
    };
    const std::array layouts{layoutView(bindings)};

    EXPECT_NO_THROW(
        Vulkan::validateDescriptorCompatibility(shader_interface, layouts));
}

TEST(DescriptorCompatibilityTests, AcceptsEqualDescriptorArrayCount) {
    const auto shader_interface = makeInterface(
        Shader::ShaderStage::Fragment,
        {
            requiredBinding(kSetZero, kBindingZero,
                            Shader::ShaderResourceType::CombinedImageSampler,
                            kTwoDescriptors),
        });

    const std::array bindings{
        providedBinding(kBindingZero, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        kFragmentStage, kTwoDescriptors),
    };
    const std::array layouts{layoutView(bindings)};

    EXPECT_NO_THROW(
        Vulkan::validateDescriptorCompatibility(shader_interface, layouts));
}

TEST(DescriptorCompatibilityTests, AcceptsBindingInSecondDescriptorSet) {
    const auto shader_interface = makeInterface(
        Shader::ShaderStage::Fragment,
        {
            requiredBinding(kSetOne, kBindingTwo,
                            Shader::ShaderResourceType::CombinedImageSampler),
        });

    const std::array second_set_bindings{
        providedBinding(kBindingTwo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        kFragmentStage),
    };
    const std::array layouts{
        layoutView({}),
        layoutView(second_set_bindings),
    };

    EXPECT_NO_THROW(
        Vulkan::validateDescriptorCompatibility(shader_interface, layouts));
}

TEST(DescriptorCompatibilityTests, AcceptsAdditionalUnusedBindings) {
    const auto shader_interface = makeInterface(
        Shader::ShaderStage::Vertex,
        {
            requiredBinding(kSetZero, kBindingOne,
                            Shader::ShaderResourceType::UniformBuffer),
        });

    const std::array bindings{
        providedBinding(kBindingZero, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        kFragmentStage),
        providedBinding(kBindingOne, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        kVertexStage),
        providedBinding(kBindingTwo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        kFragmentStage),
    };
    const std::array layouts{layoutView(bindings)};

    EXPECT_NO_THROW(
        Vulkan::validateDescriptorCompatibility(shader_interface, layouts));
}

TEST(DescriptorCompatibilityTests, AllowsSameBindingIndexInDifferentSets) {
    const auto shader_interface = makeInterface(
        Shader::ShaderStage::Vertex,
        {
            requiredBinding(kSetZero, kBindingZero,
                            Shader::ShaderResourceType::UniformBuffer),
            requiredBinding(kSetOne, kBindingZero,
                            Shader::ShaderResourceType::StorageBuffer),
        });

    const std::array first_bindings{
        providedBinding(kBindingZero, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        kVertexStage),
    };
    const std::array second_bindings{
        providedBinding(kBindingZero, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        kVertexStage),
    };
    const std::array layouts{
        layoutView(first_bindings),
        layoutView(second_bindings),
    };

    EXPECT_NO_THROW(
        Vulkan::validateDescriptorCompatibility(shader_interface, layouts));
}

TEST(DescriptorCompatibilityTests, AcceptsSharedVertexAndFragmentBinding) {
    const auto vertex_interface = makeInterface(
        Shader::ShaderStage::Vertex,
        {
            requiredBinding(kSetZero, kBindingZero,
                            Shader::ShaderResourceType::UniformBuffer),
        });

    const auto fragment_interface = makeInterface(
        Shader::ShaderStage::Fragment,
        {
            requiredBinding(kSetZero, kBindingZero,
                            Shader::ShaderResourceType::UniformBuffer),
        });

    const std::array bindings{
        providedBinding(kBindingZero, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        kVertexFragmentStages),
    };
    const std::array layouts{layoutView(bindings)};

    EXPECT_NO_THROW(
        Vulkan::validateDescriptorCompatibility(vertex_interface, layouts));
    EXPECT_NO_THROW(
        Vulkan::validateDescriptorCompatibility(fragment_interface, layouts));
}

TEST(DescriptorCompatibilityTests, RejectsMissingDescriptorSet) {
    const auto shader_interface = makeInterface(
        Shader::ShaderStage::Fragment,
        {
            requiredBinding(kSetOne, kBindingZero,
                            Shader::ShaderResourceType::CombinedImageSampler),
        });

    const std::array layouts{layoutView({})};

    expectIncompatible(
        shader_interface, layouts,
        "Pipeline layout does not contain the required shader descriptor set");
}

TEST(DescriptorCompatibilityTests, RejectsMissingDescriptorBinding) {
    const auto shader_interface = makeInterface(
        Shader::ShaderStage::Vertex,
        {
            requiredBinding(kSetZero, kBindingTwo,
                            Shader::ShaderResourceType::UniformBuffer),
        });

    const std::array bindings{
        providedBinding(kBindingZero, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        kVertexStage),
        providedBinding(kBindingOne, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        kVertexStage),
    };
    const std::array layouts{layoutView(bindings)};

    expectIncompatible(
        shader_interface, layouts,
        "Pipeline descriptor set does not contain the required shader binding");
}

TEST(DescriptorCompatibilityTests, RejectsRequiredBindingInEmptySet) {
    const auto shader_interface = makeInterface(
        Shader::ShaderStage::Vertex,
        {
            requiredBinding(kSetZero, kBindingZero,
                            Shader::ShaderResourceType::UniformBuffer),
        });

    const std::array layouts{layoutView({})};

    expectIncompatible(
        shader_interface, layouts,
        "Pipeline descriptor set does not contain the required shader binding");
}

TEST(DescriptorCompatibilityTests, RejectsDescriptorTypeMismatch) {
    const auto shader_interface = makeInterface(
        Shader::ShaderStage::Fragment,
        {
            requiredBinding(kSetZero, kBindingZero,
                            Shader::ShaderResourceType::CombinedImageSampler),
        });

    const std::array bindings{
        providedBinding(kBindingZero, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        kFragmentStage),
    };
    const std::array layouts{layoutView(bindings)};

    expectIncompatible(
        shader_interface, layouts,
        "Pipeline descriptor binding type does not match shader requirement");
}

TEST(DescriptorCompatibilityTests, RejectsInsufficientDescriptorCount) {
    const auto shader_interface = makeInterface(
        Shader::ShaderStage::Fragment,
        {
            requiredBinding(kSetZero, kBindingZero,
                            Shader::ShaderResourceType::CombinedImageSampler,
                            kFourDescriptors),
        });

    const std::array bindings{
        providedBinding(kBindingZero, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        kFragmentStage, kTwoDescriptors),
    };
    const std::array layouts{layoutView(bindings)};

    expectIncompatible(
        shader_interface, layouts,
        "Pipeline descriptor binding count is smaller than shader requirement");
}

TEST(DescriptorCompatibilityTests, RejectsMissingShaderStageVisibility) {
    const auto shader_interface = makeInterface(
        Shader::ShaderStage::Fragment,
        {
            requiredBinding(kSetZero, kBindingZero,
                            Shader::ShaderResourceType::UniformBuffer),
        });

    const std::array bindings{
        providedBinding(kBindingZero, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        kVertexStage),
    };
    const std::array layouts{layoutView(bindings)};

    expectIncompatible(shader_interface, layouts,
                       "Pipeline descriptor binding does not include the "
                       "required shader stage");
}

TEST(DescriptorCompatibilityTests, RejectsZeroShaderDescriptorCount) {
    const auto shader_interface = makeInterface(
        Shader::ShaderStage::Vertex,
        {
            requiredBinding(kSetZero, kBindingZero,
                            Shader::ShaderResourceType::UniformBuffer, 0U),
        });

    ASSERT_DEATH(Vulkan::validateDescriptorCompatibility(shader_interface, {}),
                 "Shader descriptor requirement must have a nonzero "
                 "descriptor count");
}

TEST(DescriptorCompatibilityTests, RejectsZeroPipelineDescriptorCount) {
    const Shader::ShaderInterface shader_interface{};

    const std::array bindings{
        providedBinding(kBindingZero, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        kVertexStage, 0U),
    };
    const std::array layouts{layoutView(bindings)};

    ASSERT_DEATH(
        Vulkan::validateDescriptorCompatibility(shader_interface, layouts),
        "Pipeline descriptor binding must have a nonzero descriptor count");
}

TEST(DescriptorCompatibilityTests, RejectsDuplicateBindingIndices) {
    const Shader::ShaderInterface shader_interface{};

    const std::array bindings{
        providedBinding(kBindingOne, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        kVertexStage),
        providedBinding(kBindingOne, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        kFragmentStage),
    };
    const std::array layouts{layoutView(bindings)};

    ASSERT_DEATH(
        Vulkan::validateDescriptorCompatibility(shader_interface, layouts),
        "Pipeline descriptor-set layout contains duplicate binding indices");
}
