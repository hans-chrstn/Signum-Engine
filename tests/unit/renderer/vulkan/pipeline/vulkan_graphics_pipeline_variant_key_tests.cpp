#include "engine/renderer/pipeline/cull_mode.hpp"
#include "engine/renderer/shader/graphics_shader_data.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_graphics_pipeline_variant_key.hpp"
#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <vulkan/vulkan.h>

namespace Renderer = SNE::Engine::Renderer;
namespace Shader = SNE::Engine::Renderer::Shader;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    constexpr std::uint32_t kVertexWord{101U};
    constexpr std::uint32_t kFragmentWord{202U};
    constexpr std::uint32_t kVertexStride{24U};
    constexpr std::uint32_t kPushConstantSize{64U};

    [[nodiscard]] auto makeShaders() -> Shader::GraphicsShaderData {
        return {
            .vertex =
                {
                    .artifact = {.words = {kVertexWord}},
                    .selection =
                        {
                            .stage = Shader::ShaderStage::Vertex,
                            .entry_point = "main",
                        },
                },
            .fragment =
                {
                    .artifact = {.words = {kFragmentWord}},
                    .selection =
                        {
                            .stage = Shader::ShaderStage::Fragment,
                            .entry_point = "main",
                        },
                },
        };
    }

    [[nodiscard]] auto makeKey() -> Vulkan::VulkanGraphicsPipelineVariantKey {
        const Shader::GraphicsShaderData shaders = makeShaders();

        constexpr std::array<VkVertexInputBindingDescription, 1> bindings{
            VkVertexInputBindingDescription{
                .binding = 0U,
                .stride = kVertexStride,
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            },
        };

        constexpr std::array<VkVertexInputAttributeDescription, 1> attributes{
            VkVertexInputAttributeDescription{
                .location = 0U,
                .binding = 0U,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = 0U,
            },
        };

        constexpr std::array<VkPushConstantRange, 1> ranges{
            VkPushConstantRange{
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .offset = 0U,
                .size = kPushConstantSize,
            },
        };

        return Vulkan::makeVulkanGraphicsPipelineVariantKey(
            VK_NULL_HANDLE,
            {
                .color = VK_FORMAT_R8G8B8A8_UNORM,
                .depth = VK_FORMAT_D32_SFLOAT,
            },
            shaders,
            {
                .bindings = bindings,
                .attributes = attributes,
            },
            {
                .push_constant_ranges = ranges,
                .descriptor_set_layouts = {},
            },
            {});
    }
} // namespace

TEST(VulkanGraphicsPipelineVariantKeyTests,
     IndependentlyConstructedEquivalentKeysCompareEqual) {
    const auto first = makeKey();
    const auto second = makeKey();

    EXPECT_TRUE(first == second);
}

TEST(VulkanGraphicsPipelineVariantKeyTests,
     DifferentShaderConfigurationsCompareUnequal) {
    const auto original = makeKey();
    auto modified = makeKey();

    modified.shaders.vertex.artifact.words[0] = kFragmentWord;
    EXPECT_FALSE(original == modified);

    modified = makeKey();
    modified.shaders.fragment.artifact.words[0] = kVertexWord;
    EXPECT_FALSE(original == modified);

    modified = makeKey();
    modified.shaders.vertex.selection.entry_point = "alternative";
    EXPECT_FALSE(original == modified);
}

TEST(VulkanGraphicsPipelineVariantKeyTests,
     DifferentGraphicsStatesCompareUnequal) {
    const auto original = makeKey();
    auto modified = makeKey();

    modified.state.cull_mode = Renderer::CullMode::Back;
    EXPECT_FALSE(original == modified);

    modified = makeKey();
    modified.state.depth_write_enabled = false;
    EXPECT_FALSE(original == modified);

    modified = makeKey();
    modified.state.color_blend.enabled = true;
    EXPECT_FALSE(original == modified);
}

TEST(VulkanGraphicsPipelineVariantKeyTests,
     DifferentVertexBindingsCompareUnequal) {
    const auto original = makeKey();
    auto modified = makeKey();

    modified.vertex_bindings[0].binding = 1U;
    EXPECT_FALSE(original == modified);

    modified = makeKey();
    constexpr std::uint32_t stride = 32U;
    modified.vertex_bindings[0].stride = stride;
    EXPECT_FALSE(original == modified);

    modified = makeKey();
    modified.vertex_bindings[0].input_rate = VK_VERTEX_INPUT_RATE_INSTANCE;
    EXPECT_FALSE(original == modified);
}

TEST(VulkanGraphicsPipelineVariantKeyTests,
     DifferentVertexAttributesCompareUnequal) {
    const auto original = makeKey();
    auto modified = makeKey();

    modified.vertex_attributes[0].location = 1U;
    EXPECT_FALSE(original == modified);

    modified = makeKey();
    modified.vertex_attributes[0].binding = 1U;
    EXPECT_FALSE(original == modified);

    modified = makeKey();
    modified.vertex_attributes[0].format = VK_FORMAT_R32G32_SFLOAT;
    EXPECT_FALSE(original == modified);

    modified = makeKey();
    modified.vertex_attributes[0].offset = 4U;
    EXPECT_FALSE(original == modified);
}

TEST(VulkanGraphicsPipelineVariantKeyTests,
     DifferentPushConstantRangesCompareUnequal) {
    const auto original = makeKey();
    auto modified = makeKey();

    modified.push_constant_ranges[0].stage_flags = VK_SHADER_STAGE_FRAGMENT_BIT;
    EXPECT_FALSE(original == modified);

    modified = makeKey();
    modified.push_constant_ranges[0].offset = 4U;
    EXPECT_FALSE(original == modified);

    modified = makeKey();
    constexpr std::uint32_t size = 32U;
    modified.push_constant_ranges[0].size = size;
    EXPECT_FALSE(original == modified);
}

TEST(VulkanGraphicsPipelineVariantKeyTests,
     DifferentAttachmentFormatsCompareUnequal) {
    const auto original = makeKey();
    auto modified = makeKey();

    modified.attachment_formats.color = VK_FORMAT_B8G8R8A8_UNORM;
    EXPECT_FALSE(original == modified);

    modified = makeKey();
    modified.attachment_formats.depth = VK_FORMAT_D16_UNORM;
    EXPECT_FALSE(original == modified);
}

TEST(VulkanGraphicsPipelineVariantKeyTests,
     DifferentDescriptorLayoutSequencesCompareUnequal) {
    const auto original = makeKey();
    auto modified = makeKey();

    modified.descriptor_set_layouts.push_back(VK_NULL_HANDLE);

    EXPECT_FALSE(original == modified);
}

TEST(VulkanGraphicsPipelineVariantKeyTests,
     CopiesBorrowedInputsIntoOwnedStorage) {
    Shader::GraphicsShaderData shaders = makeShaders();

    std::array<VkVertexInputBindingDescription, 1> bindings{
        VkVertexInputBindingDescription{
            .binding = 0U,
            .stride = kVertexStride,
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        },
    };

    std::array<VkVertexInputAttributeDescription, 1> attributes{
        VkVertexInputAttributeDescription{
            .location = 0U,
            .binding = 0U,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = 0U,
        },
    };

    std::array<VkPushConstantRange, 1> ranges{
        VkPushConstantRange{
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .offset = 0U,
            .size = kPushConstantSize,
        },
    };

    const auto key = Vulkan::makeVulkanGraphicsPipelineVariantKey(
        VK_NULL_HANDLE,
        {
            .color = VK_FORMAT_R8G8B8A8_UNORM,
            .depth = VK_FORMAT_D32_SFLOAT,
        },
        shaders,
        {
            .bindings = bindings,
            .attributes = attributes,
        },
        {
            .push_constant_ranges = ranges,
            .descriptor_set_layouts = {},
        },
        {});

    constexpr std::uint32_t stride = 32U;
    constexpr std::uint32_t size = 32U;

    shaders.vertex.artifact.words[0] = kFragmentWord;
    bindings[0].stride = stride;
    attributes[0].location = 1U;
    ranges[0].size = size;

    EXPECT_EQ(key.shaders.vertex.artifact.words[0], kVertexWord);
    EXPECT_EQ(key.vertex_bindings[0].stride, kVertexStride);
    EXPECT_EQ(key.vertex_attributes[0].location, 0U);
    EXPECT_EQ(key.push_constant_ranges[0].size, kPushConstantSize);
}

TEST(VulkanGraphicsPipelineVariantKeyTests,
     RejectsNullDescriptorLayoutReferences) {
    const Shader::GraphicsShaderData shaders = makeShaders();

    const std::array<const Vulkan::VulkanDescriptorSetLayout *, 1> layouts{
        nullptr,
    };

    EXPECT_DEATH(static_cast<void>(Vulkan::makeVulkanGraphicsPipelineVariantKey(
                     VK_NULL_HANDLE, {}, shaders, {},
                     {
                         .push_constant_ranges = {},
                         .descriptor_set_layouts = layouts,
                     },
                     {})),
                 "requires non-null descriptor-set layout references");
}
