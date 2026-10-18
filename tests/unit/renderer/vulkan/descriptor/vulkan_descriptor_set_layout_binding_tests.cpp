#include "engine/renderer/shader/shader_descriptor_binding.hpp"
#include "engine/renderer/shader/shader_resource_type.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include "engine/renderer/vulkan/descriptor/vulkan_descriptor_set_layout.hpp"
#include <gtest/gtest.h>
#include <vulkan/vulkan.h>

namespace Shader = SNE::Engine::Renderer::Shader;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

TEST(VulkanDescriptorSetLayoutBindingTests, ConvertsCombinedImageSampler) {
    const Shader::ShaderDescriptorBinding requirement{
        .set = 0U,
        .binding = 0U,
        .type = Shader::ShaderResourceType::CombinedImageSampler,
        .count = 1U,
    };

    const VkDescriptorSetLayoutBinding result =
        Vulkan::makeVkDescriptorSetLayoutBinding(requirement,
                                                 Shader::ShaderStage::Fragment);

    EXPECT_EQ(result.binding, 0U);
    EXPECT_EQ(result.descriptorType, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    EXPECT_EQ(result.descriptorCount, 1U);
    EXPECT_EQ(result.stageFlags, VK_SHADER_STAGE_FRAGMENT_BIT);
    EXPECT_EQ(result.pImmutableSamplers, nullptr);
}

TEST(VulkanDescriptorSetLayoutBindingTests, ConvertsUniformBuffer) {
    const Shader::ShaderDescriptorBinding requirement{
        .set = 0U,
        .binding = 1U,
        .type = Shader::ShaderResourceType::UniformBuffer,
        .count = 1U,
    };

    const VkDescriptorSetLayoutBinding result =
        Vulkan::makeVkDescriptorSetLayoutBinding(requirement,
                                                 Shader::ShaderStage::Vertex);

    EXPECT_EQ(result.binding, 1U);
    EXPECT_EQ(result.descriptorType, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    EXPECT_EQ(result.descriptorCount, 1U);
    EXPECT_EQ(result.stageFlags, VK_SHADER_STAGE_VERTEX_BIT);
    EXPECT_EQ(result.pImmutableSamplers, nullptr);
}

TEST(VulkanDescriptorSetLayoutBindingTests, ConvertsStorageBuffer) {
    const Shader::ShaderDescriptorBinding requirement{
        .set = 0U,
        .binding = 2U,
        .type = Shader::ShaderResourceType::StorageBuffer,
        .count = 1U,
    };

    const VkDescriptorSetLayoutBinding result =
        Vulkan::makeVkDescriptorSetLayoutBinding(requirement,
                                                 Shader::ShaderStage::Compute);

    EXPECT_EQ(result.binding, 2U);
    EXPECT_EQ(result.descriptorType, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    EXPECT_EQ(result.descriptorCount, 1U);
    EXPECT_EQ(result.stageFlags, VK_SHADER_STAGE_COMPUTE_BIT);
    EXPECT_EQ(result.pImmutableSamplers, nullptr);
}

TEST(VulkanDescriptorSetLayoutBindingTests, PreservesDescriptorArrayCount) {
    const Shader::ShaderDescriptorBinding requirement{
        .set = 1U,
        .binding = 3U,
        .type = Shader::ShaderResourceType::UniformBuffer,
        .count = 4U,
    };

    const VkDescriptorSetLayoutBinding result =
        Vulkan::makeVkDescriptorSetLayoutBinding(requirement,
                                                 Shader::ShaderStage::Vertex);

    EXPECT_EQ(result.binding, 3U);
    EXPECT_EQ(result.descriptorCount, 4U);
    EXPECT_EQ(result.descriptorType, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    EXPECT_EQ(result.stageFlags, VK_SHADER_STAGE_VERTEX_BIT);
}

TEST(VulkanDescriptorSetLayoutBindingTests, RejectsZeroDescriptorCount) {
    const Shader::ShaderDescriptorBinding requirement{
        .set = 0U,
        .binding = 0U,
        .type = Shader::ShaderResourceType::UniformBuffer,
        .count = 0U,
    };

    ASSERT_DEATH(static_cast<void>(Vulkan::makeVkDescriptorSetLayoutBinding(
                     requirement, Shader::ShaderStage::Vertex)),
                 "Vulkan descriptor-set layout binding requires a nonzero "
                 "descriptor count");
}
