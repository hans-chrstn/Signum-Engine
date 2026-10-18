#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/renderer/shader/shader_descriptor_binding.hpp"
#include "engine/renderer/shader/shader_interface.hpp"
#include "engine/renderer/shader/shader_resource_type.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include "engine/renderer/vulkan/descriptor/vulkan_descriptor_set_layout.hpp"
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
    [[nodiscard]] constexpr auto
    makeRequirement(std::uint32_t set, std::uint32_t binding,
                    Shader::ShaderResourceType type, std::uint32_t count = 1U)
        -> Shader::ShaderDescriptorBinding {
        return {
            .set = set,
            .binding = binding,
            .type = type,
            .count = count,
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

    auto expectIncompatible(std::span<const Shader::ShaderInterface> interfaces)
        -> void {
        try {
            static_cast<void>(
                Vulkan::mergeVkDescriptorSetLayoutRequirements(interfaces));

            FAIL() << "Expected incompatible descriptor requirements";
        } catch (const Core::Error::EngineError &error) {
            EXPECT_EQ(error.getCode(),
                      Core::Error::Code::VulkanDescriptorLayoutIncompatible);

            EXPECT_STREQ(
                error.what(),
                "Shader descriptor requirements have incompatible types "
                "or counts at the same descriptor location");

            EXPECT_EQ(error.getOperation(),
                      "Merge Vulkan Descriptor Set Layout Requirements");
        }
    }
} // namespace

TEST(VulkanDescriptorSetLayoutRequirementsTests, AcceptsEmptyInterfaces) {
    const std::array<Shader::ShaderInterface, 0> interfaces{};

    const auto groups =
        Vulkan::mergeVkDescriptorSetLayoutRequirements(interfaces);

    EXPECT_TRUE(groups.empty());
}

TEST(VulkanDescriptorSetLayoutRequirementsTests,
     AcceptsInterfacesWithoutDescriptors) {
    const std::array interfaces{
        makeInterface(Shader::ShaderStage::Vertex, {}),
        makeInterface(Shader::ShaderStage::Fragment, {}),
    };

    const auto groups =
        Vulkan::mergeVkDescriptorSetLayoutRequirements(interfaces);

    EXPECT_TRUE(groups.empty());
}

TEST(VulkanDescriptorSetLayoutRequirementsTests,
     GroupsAndSortsDescriptorRequirements) {
    const std::array interfaces{
        makeInterface(
            Shader::ShaderStage::Vertex,
            {
                makeRequirement(2U, 4U,
                                Shader::ShaderResourceType::UniformBuffer),
                makeRequirement(0U, 3U,
                                Shader::ShaderResourceType::UniformBuffer),
                makeRequirement(0U, 1U,
                                Shader::ShaderResourceType::StorageBuffer, 2U),
            }),
    };

    const auto groups =
        Vulkan::mergeVkDescriptorSetLayoutRequirements(interfaces);

    ASSERT_EQ(groups.size(), 2U);

    EXPECT_EQ(groups[0].set, 0U);
    EXPECT_EQ(groups[1].set, 2U);

    ASSERT_EQ(groups[0].bindings.size(), 2U);
    ASSERT_EQ(groups[1].bindings.size(), 1U);

    EXPECT_EQ(groups[0].bindings[0].binding, 1U);
    EXPECT_EQ(groups[0].bindings[0].descriptorType,
              VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    EXPECT_EQ(groups[0].bindings[0].descriptorCount, 2U);
    EXPECT_EQ(groups[0].bindings[0].stageFlags, VK_SHADER_STAGE_VERTEX_BIT);

    EXPECT_EQ(groups[0].bindings[1].binding, 3U);
    EXPECT_EQ(groups[0].bindings[1].descriptorType,
              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    EXPECT_EQ(groups[1].bindings[0].binding, 4U);
    EXPECT_EQ(groups[1].bindings[0].descriptorType,
              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
}

TEST(VulkanDescriptorSetLayoutRequirementsTests,
     MergesSharedShaderStageVisibility) {
    const std::array interfaces{
        makeInterface(
            Shader::ShaderStage::Vertex,
            {
                makeRequirement(0U, 1U,
                                Shader::ShaderResourceType::UniformBuffer, 2U),
            }),
        makeInterface(
            Shader::ShaderStage::Fragment,
            {
                makeRequirement(0U, 1U,
                                Shader::ShaderResourceType::UniformBuffer, 2U),
            }),
    };

    const auto groups =
        Vulkan::mergeVkDescriptorSetLayoutRequirements(interfaces);

    ASSERT_EQ(groups.size(), 1U);
    ASSERT_EQ(groups[0].bindings.size(), 1U);

    const VkDescriptorSetLayoutBinding &binding = groups[0].bindings[0];

    EXPECT_EQ(binding.binding, 1U);
    EXPECT_EQ(binding.descriptorType, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    EXPECT_EQ(binding.descriptorCount, 2U);
    EXPECT_EQ(binding.stageFlags,
              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    EXPECT_EQ(binding.pImmutableSamplers, nullptr);
}

TEST(VulkanDescriptorSetLayoutRequirementsTests,
     PreservesSeparateBindingsAcrossShaderStages) {
    const std::array interfaces{
        makeInterface(
            Shader::ShaderStage::Vertex,
            {
                makeRequirement(0U, 1U,
                                Shader::ShaderResourceType::UniformBuffer),
            }),
        makeInterface(
            Shader::ShaderStage::Fragment,
            {
                makeRequirement(
                    0U, 0U, Shader::ShaderResourceType::CombinedImageSampler),
            }),
    };

    const auto groups =
        Vulkan::mergeVkDescriptorSetLayoutRequirements(interfaces);

    ASSERT_EQ(groups.size(), 1U);
    ASSERT_EQ(groups[0].bindings.size(), 2U);

    EXPECT_EQ(groups[0].bindings[0].binding, 0U);
    EXPECT_EQ(groups[0].bindings[0].descriptorType,
              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    EXPECT_EQ(groups[0].bindings[0].stageFlags, VK_SHADER_STAGE_FRAGMENT_BIT);

    EXPECT_EQ(groups[0].bindings[1].binding, 1U);
    EXPECT_EQ(groups[0].bindings[1].descriptorType,
              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    EXPECT_EQ(groups[0].bindings[1].stageFlags, VK_SHADER_STAGE_VERTEX_BIT);
}

TEST(VulkanDescriptorSetLayoutRequirementsTests,
     RejectsConflictingDescriptorTypes) {
    const std::array interfaces{
        makeInterface(
            Shader::ShaderStage::Vertex,
            {
                makeRequirement(0U, 1U,
                                Shader::ShaderResourceType::UniformBuffer),
            }),
        makeInterface(
            Shader::ShaderStage::Fragment,
            {
                makeRequirement(0U, 1U,
                                Shader::ShaderResourceType::StorageBuffer),
            }),
    };

    expectIncompatible(interfaces);
}

TEST(VulkanDescriptorSetLayoutRequirementsTests,
     RejectsConflictingDescriptorCounts) {
    const std::array interfaces{
        makeInterface(
            Shader::ShaderStage::Vertex,
            {
                makeRequirement(0U, 1U,
                                Shader::ShaderResourceType::UniformBuffer, 1U),
            }),
        makeInterface(
            Shader::ShaderStage::Fragment,
            {
                makeRequirement(0U, 1U,
                                Shader::ShaderResourceType::UniformBuffer, 4U),
            }),
    };

    expectIncompatible(interfaces);
}

TEST(VulkanDescriptorSetLayoutRequirementsTests, RejectsZeroDescriptorCount) {
    const std::array interfaces{
        makeInterface(
            Shader::ShaderStage::Vertex,
            {
                makeRequirement(0U, 1U,
                                Shader::ShaderResourceType::UniformBuffer, 0U),
            }),
    };

    ASSERT_DEATH(
        static_cast<void>(
            Vulkan::mergeVkDescriptorSetLayoutRequirements(interfaces)),
        "Vulkan descriptor-set layout binding requires a nonzero "
        "descriptor count");
}
