#include "../vulkan_integration_test_fixture.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/math/matrix4.hpp"
#include "engine/renderer/pipeline/cull_mode.hpp"
#include "engine/renderer/shader/graphics_shader_data.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include "engine/renderer/vulkan/descriptor/vulkan_descriptor_set_layout.hpp"
#include "engine/renderer/vulkan/device/vulkan_depth_format.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_graphics_pipeline_cache.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_graphics_pipeline_variant_key.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_spirv.hpp"
#include <array>
#include <cstdint>
#include <filesystem>
#include <gtest/gtest.h>
#include <optional>
#include <span>
#include <vulkan/vulkan.h>

namespace Math = SNE::Engine::Core::Math;
namespace Core = SNE::Engine::Core;
namespace Renderer = SNE::Engine::Renderer;
namespace Shader = SNE::Engine::Renderer::Shader;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    constexpr VkFormat kColorFormat = VK_FORMAT_R8G8B8A8_UNORM;
    constexpr std::uint32_t kVertexStride = 24U;
    constexpr std::uint32_t kColorOffset = 12U;

    struct PipelineInputs {
        Shader::GraphicsShaderData shaders;

        std::array<VkVertexInputBindingDescription, 1> bindings{
            VkVertexInputBindingDescription{
                .binding = 0U,
                .stride = kVertexStride,
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            },
        };

        std::array<VkVertexInputAttributeDescription, 2> attributes{
            VkVertexInputAttributeDescription{
                .location = 0U,
                .binding = 0U,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = 0U,
            },
            VkVertexInputAttributeDescription{
                .location = 1U,
                .binding = 0U,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = kColorOffset,
            },
        };

        std::array<VkPushConstantRange, 1> push_constant_ranges{
            VkPushConstantRange{
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .offset = 0U,
                .size = static_cast<std::uint32_t>(sizeof(Math::Matrix4f)),
            },
        };

        [[nodiscard]] auto vertexInput() const
            -> Vulkan::GraphicsVertexInputData {
            return {
                .bindings = bindings,
                .attributes = attributes,
            };
        }

        [[nodiscard]] auto layoutInput(
            std::span<const Vulkan::VulkanDescriptorSetLayout *const> layouts =
                {}) const -> Vulkan::GraphicsPipelineLayoutData {
            return {
                .push_constant_ranges = push_constant_ranges,
                .descriptor_set_layouts = layouts,
            };
        }
    };

    [[nodiscard]] auto makeShaders() -> Shader::GraphicsShaderData {
        const auto shader_directory =
            std::filesystem::path{SIGNUM_TEST_SHADER_DIR};

        return {
            .vertex =
                {
                    .artifact = Vulkan::loadSpirv(shader_directory /
                                                  "triangle.vert.spv"),
                    .selection =
                        {
                            .stage = Shader::ShaderStage::Vertex,
                            .entry_point = "main",
                        },
                },
            .fragment =
                {
                    .artifact = Vulkan::loadSpirv(shader_directory /
                                                  "triangle.frag.spv"),
                    .selection =
                        {
                            .stage = Shader::ShaderStage::Fragment,
                            .entry_point = "main",
                        },
                },
        };
    }

    [[nodiscard]] auto makeInputs() -> PipelineInputs {
        return {
            .shaders = makeShaders(),
        };
    }

    class VulkanGraphicsPipelineCacheIntegrationTest
        : public VulkanIntegrationTest {
      protected:
        VkFormat m_DepthFormat{VK_FORMAT_UNDEFINED};
        std::uint32_t m_MaxPushConstantsSize{};

        auto SetUp() -> void override {
            VulkanIntegrationTest::SetUp();

            if (HasFatalFailure() || IsSkipped()) {
                return;
            }

            if (!m_DynamicRenderingEnabled) {
                GTEST_SKIP() << "Dynamic rendering is unsupported";
            }

            const std::optional<VkFormat> depth_format =
                Vulkan::selectSupportedDepthFormat(m_PhysicalDevice);

            if (!depth_format.has_value()) {
                GTEST_SKIP() << "No supported depth attachment format";
            }

            m_DepthFormat = depth_format.value();

            VkFormatProperties format_properties{};
            vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, kColorFormat,
                                                &format_properties);

            if ((format_properties.optimalTilingFeatures &
                 VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) == 0U) {
                GTEST_SKIP() << "Test color format is unsupported";
            }

            VkPhysicalDeviceProperties device_properties{};
            vkGetPhysicalDeviceProperties(m_PhysicalDevice, &device_properties);

            m_MaxPushConstantsSize =
                device_properties.limits.maxPushConstantsSize;
        }

        [[nodiscard]] auto attachmentFormats() const
            -> Vulkan::GraphicsAttachmentFormats {
            return {
                .color = kColorFormat,
                .depth = m_DepthFormat,
            };
        }

        [[nodiscard]] auto requestPipeline(
            Vulkan::VulkanGraphicsPipelineCache &cache,
            const PipelineInputs &inputs,
            Renderer::GraphicsPipelineState state = {},
            std::span<const Vulkan::VulkanDescriptorSetLayout *const> layouts =
                {}) const -> const Vulkan::VulkanGraphicsPipeline * {
            return cache.getOrCreate(m_Device, attachmentFormats(),
                                     inputs.shaders, inputs.vertexInput(),
                                     inputs.layoutInput(layouts),
                                     m_MaxPushConstantsSize, state);
        }

        [[nodiscard]] auto makeKey(
            const PipelineInputs &inputs,
            Renderer::GraphicsPipelineState state = {},
            std::span<const Vulkan::VulkanDescriptorSetLayout *const> layouts =
                {}) const -> Vulkan::VulkanGraphicsPipelineVariantKey {
            return Vulkan::makeVulkanGraphicsPipelineVariantKey(
                m_Device, attachmentFormats(), inputs.shaders,
                inputs.vertexInput(), inputs.layoutInput(layouts), state);
        }
    };
} // namespace

TEST_F(VulkanGraphicsPipelineCacheIntegrationTest,
       CacheMissCreatesAndStoresPipeline) {
    const PipelineInputs inputs = makeInputs();
    Vulkan::VulkanGraphicsPipelineCache cache;

    const auto key = makeKey(inputs);

    EXPECT_EQ(cache.find(key), nullptr);

    const Vulkan::VulkanGraphicsPipeline *pipeline =
        requestPipeline(cache, inputs);

    ASSERT_NE(pipeline, nullptr);
    EXPECT_NE(pipeline->nativeHandle(), VK_NULL_HANDLE);
    EXPECT_NE(pipeline->layoutHandle(), VK_NULL_HANDLE);
    EXPECT_EQ(cache.find(key), pipeline);
}

TEST_F(VulkanGraphicsPipelineCacheIntegrationTest,
       EquivalentRequestsReuseExistingPipeline) {
    const PipelineInputs first_inputs = makeInputs();
    const PipelineInputs second_inputs = makeInputs();

    Vulkan::VulkanGraphicsPipelineCache cache;

    const Vulkan::VulkanGraphicsPipeline *first =
        requestPipeline(cache, first_inputs);

    const Vulkan::VulkanGraphicsPipeline *second =
        requestPipeline(cache, second_inputs);

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_EQ(first, second);
    EXPECT_EQ(first->nativeHandle(), second->nativeHandle());
    EXPECT_EQ(first->layoutHandle(), second->layoutHandle());
}

TEST_F(VulkanGraphicsPipelineCacheIntegrationTest,
       DifferentPipelineStatesCreateDistinctPipelines) {
    const PipelineInputs inputs = makeInputs();
    Vulkan::VulkanGraphicsPipelineCache cache;

    const Vulkan::VulkanGraphicsPipeline *original =
        requestPipeline(cache, inputs);

    Renderer::GraphicsPipelineState alternate_state{};
    alternate_state.cull_mode = Renderer::CullMode::Back;

    const Vulkan::VulkanGraphicsPipeline *alternate =
        requestPipeline(cache, inputs, alternate_state);

    ASSERT_NE(original, nullptr);
    ASSERT_NE(alternate, nullptr);

    EXPECT_NE(original, alternate);
    EXPECT_NE(original->nativeHandle(), alternate->nativeHandle());

    EXPECT_EQ(cache.find(makeKey(inputs)), original);
    EXPECT_EQ(cache.find(makeKey(inputs, alternate_state)), alternate);
}

TEST_F(VulkanGraphicsPipelineCacheIntegrationTest,
       PreviouslyReturnedPointersRemainStableAfterInsertion) {
    const PipelineInputs inputs = makeInputs();
    Vulkan::VulkanGraphicsPipelineCache cache;

    const Vulkan::VulkanGraphicsPipeline *original =
        requestPipeline(cache, inputs);

    ASSERT_NE(original, nullptr);

    const VkPipeline original_handle = original->nativeHandle();
    const VkPipelineLayout original_layout = original->layoutHandle();

    Renderer::GraphicsPipelineState back_culling{};
    back_culling.cull_mode = Renderer::CullMode::Back;

    Renderer::GraphicsPipelineState front_culling{};
    front_culling.cull_mode = Renderer::CullMode::Front;

    Renderer::GraphicsPipelineState no_depth_write{};
    no_depth_write.depth_write_enabled = false;

    ASSERT_NE(requestPipeline(cache, inputs, back_culling), nullptr);
    ASSERT_NE(requestPipeline(cache, inputs, front_culling), nullptr);
    ASSERT_NE(requestPipeline(cache, inputs, no_depth_write), nullptr);

    EXPECT_EQ(cache.find(makeKey(inputs)), original);
    EXPECT_EQ(original->nativeHandle(), original_handle);
    EXPECT_EQ(original->layoutHandle(), original_layout);
}

TEST_F(VulkanGraphicsPipelineCacheIntegrationTest,
       CacheOwnsConfigurationAfterSourceInputsAreDestroyed) {
    Vulkan::VulkanGraphicsPipelineCache cache;

    Vulkan::VulkanGraphicsPipelineVariantKey saved_key{};

    {
        const PipelineInputs inputs = makeInputs();

        saved_key = makeKey(inputs);

        const Vulkan::VulkanGraphicsPipeline *pipeline =
            requestPipeline(cache, inputs);

        ASSERT_NE(pipeline, nullptr);
        ASSERT_EQ(cache.find(saved_key), pipeline);
    }

    const Vulkan::VulkanGraphicsPipeline *pipeline = cache.find(saved_key);

    ASSERT_NE(pipeline, nullptr);
    EXPECT_NE(pipeline->nativeHandle(), VK_NULL_HANDLE);
    EXPECT_NE(pipeline->layoutHandle(), VK_NULL_HANDLE);
}

TEST_F(VulkanGraphicsPipelineCacheIntegrationTest,
       FailedPipelineCreationPreservesExistingEntries) {
    const PipelineInputs valid_inputs = makeInputs();
    Vulkan::VulkanGraphicsPipelineCache cache;

    const Vulkan::VulkanGraphicsPipeline *original =
        requestPipeline(cache, valid_inputs);

    ASSERT_NE(original, nullptr);

    const VkPipeline original_handle = original->nativeHandle();

    PipelineInputs invalid_inputs = makeInputs();
    invalid_inputs.shaders.fragment.selection.entry_point =
        "missing_entry_point";

    const auto invalid_key = makeKey(invalid_inputs);

    EXPECT_THROW(static_cast<void>(requestPipeline(cache, invalid_inputs)),
                 Core::Error::EngineError);

    EXPECT_EQ(cache.find(invalid_key), nullptr);
    EXPECT_EQ(cache.find(makeKey(valid_inputs)), original);
    EXPECT_EQ(original->nativeHandle(), original_handle);

    const Vulkan::VulkanGraphicsPipeline *reused =
        requestPipeline(cache, valid_inputs);

    EXPECT_EQ(reused, original);
}

TEST_F(VulkanGraphicsPipelineCacheIntegrationTest,
       DifferentDescriptorSetLayoutsCreateDistinctPipelines) {
    const PipelineInputs inputs = makeInputs();

    VkDescriptorSetLayoutCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

    const Vulkan::VulkanDescriptorSetLayout first_layout{m_Device, create_info};

    const Vulkan::VulkanDescriptorSetLayout second_layout{m_Device,
                                                          create_info};

    const std::array<const Vulkan::VulkanDescriptorSetLayout *, 1>
        first_layouts{&first_layout};

    const std::array<const Vulkan::VulkanDescriptorSetLayout *, 1>
        second_layouts{&second_layout};

    Vulkan::VulkanGraphicsPipelineCache cache;

    const Vulkan::VulkanGraphicsPipeline *first =
        requestPipeline(cache, inputs, {}, first_layouts);

    const Vulkan::VulkanGraphicsPipeline *second =
        requestPipeline(cache, inputs, {}, second_layouts);

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_NE(first, second);
    EXPECT_NE(first->nativeHandle(), second->nativeHandle());

    EXPECT_EQ(cache.find(makeKey(inputs, {}, first_layouts)), first);
    EXPECT_EQ(cache.find(makeKey(inputs, {}, second_layouts)), second);
}
