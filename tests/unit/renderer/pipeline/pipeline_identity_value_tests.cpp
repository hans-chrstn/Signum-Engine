#include "engine/renderer/geometry/vertex_attribute_description.hpp"
#include "engine/renderer/shader/push_constant_range.hpp"
#include "engine/renderer/shader/shader_descriptor_binding.hpp"
#include "engine/renderer/shader/shader_resource_type.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include "engine/renderer/vulkan/descriptor/vulkan_descriptor_set_layout.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_graphics_pipeline.hpp"
#include <gtest/gtest.h>
#include <vulkan/vulkan.h>

namespace Renderer = SNE::Engine::Renderer;
namespace Shader = SNE::Engine::Renderer::Shader;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

TEST(PipelineIdentityValueTests, ComparesVertexAttributeDescriptions) {
    const Renderer::VertexAttributeDescription original{
        .location = 0U,
        .format = Renderer::VertexFormat::Float32x2,
        .offset = 0U,
    };

    const Renderer::VertexAttributeDescription equivalent{
        .location = 0U,
        .format = Renderer::VertexFormat::Float32x2,
        .offset = 0U,
    };

    EXPECT_TRUE(original == equivalent);

    Renderer::VertexAttributeDescription modified = original;

    modified.location = 1U;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.format = Renderer::VertexFormat::Float32x3;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.offset = 4U;
    EXPECT_FALSE(original == modified);
}

TEST(PipelineIdentityValueTests, ComparesPushConstantRanges) {
    const Shader::PushConstantRange original{
        .offset = 0U,
        .size = 16U,
        .stage = Shader::ShaderStage::Vertex,
    };

    const Shader::PushConstantRange equivalent{
        .offset = 0U,
        .size = 16U,
        .stage = Shader::ShaderStage::Vertex,
    };

    EXPECT_TRUE(original == equivalent);

    Shader::PushConstantRange modified = original;

    modified.offset = 4U;
    EXPECT_FALSE(original == modified);

    constexpr std::uint32_t size = 32U;

    modified = original;
    modified.size = size;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.stage = Shader::ShaderStage::Fragment;
    EXPECT_FALSE(original == modified);
}

TEST(PipelineIdentityValueTests, ComparesShaderDescriptorBindings) {
    const Shader::ShaderDescriptorBinding original{
        .set = 0U,
        .binding = 0U,
        .type = Shader::ShaderResourceType::CombinedImageSampler,
        .count = 1U,
    };

    const Shader::ShaderDescriptorBinding equivalent{
        .set = 0U,
        .binding = 0U,
        .type = Shader::ShaderResourceType::CombinedImageSampler,
        .count = 1U,
    };

    EXPECT_TRUE(original == equivalent);

    Shader::ShaderDescriptorBinding modified = original;

    modified.set = 1U;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.binding = 1U;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.type = Shader::ShaderResourceType::UniformBuffer;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.count = 2U;
    EXPECT_FALSE(original == modified);
}

TEST(PipelineIdentityValueTests, ComparesVulkanDescriptorBindings) {
    const Vulkan::VulkanDescriptorBindingDescription original{
        .binding = 0U,
        .descriptor_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptor_count = 1U,
        .stage_flags = VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    const Vulkan::VulkanDescriptorBindingDescription equivalent{
        .binding = 0U,
        .descriptor_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptor_count = 1U,
        .stage_flags = VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    EXPECT_TRUE(original == equivalent);

    Vulkan::VulkanDescriptorBindingDescription modified = original;

    modified.binding = 1U;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.descriptor_count = 2U;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.stage_flags = VK_SHADER_STAGE_VERTEX_BIT;
    EXPECT_FALSE(original == modified);
}

TEST(PipelineIdentityValueTests, ComparesAttachmentFormats) {
    const Vulkan::GraphicsAttachmentFormats original{
        .color = VK_FORMAT_R8G8B8A8_UNORM,
        .depth = VK_FORMAT_D32_SFLOAT,
    };

    const Vulkan::GraphicsAttachmentFormats equivalent{
        .color = VK_FORMAT_R8G8B8A8_UNORM,
        .depth = VK_FORMAT_D32_SFLOAT,
    };

    EXPECT_TRUE(original == equivalent);

    Vulkan::GraphicsAttachmentFormats modified = original;

    modified.color = VK_FORMAT_B8G8R8A8_UNORM;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.depth = VK_FORMAT_D16_UNORM;
    EXPECT_FALSE(original == modified);
}
