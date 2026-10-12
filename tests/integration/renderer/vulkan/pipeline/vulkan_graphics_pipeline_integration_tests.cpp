#include "../vulkan_integration_test_fixture.hpp"
#include "engine/core/math/matrix4.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include "engine/renderer/vulkan/device/vulkan_depth_format.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_graphics_pipeline.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_spirv.hpp"
#include <array>
#include <cstdint>
#include <filesystem>
#include <gtest/gtest.h>
#include <optional>
#include <type_traits>
#include <utility>
#include <vulkan/vulkan.h>

namespace Math = SNE::Engine::Core::Math;
namespace Shader = SNE::Engine::Renderer::Shader;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    constexpr VkFormat kColorFormat = VK_FORMAT_R8G8B8A8_UNORM;
    constexpr std::uint32_t kVertexStride = 24U;
    constexpr std::uint32_t kColorOffset = 12U;
    constexpr std::uint32_t kTestPushConstantLimit = 128U;

    static_assert(
        !std::is_copy_constructible_v<Vulkan::VulkanGraphicsPipeline>);
    static_assert(!std::is_copy_assignable_v<Vulkan::VulkanGraphicsPipeline>);
    static_assert(
        std::is_nothrow_move_constructible_v<Vulkan::VulkanGraphicsPipeline>);
    static_assert(
        std::is_nothrow_move_assignable_v<Vulkan::VulkanGraphicsPipeline>);

    [[nodiscard]] auto makeDepthPipeline(VkDevice device,
                                         VkPhysicalDevice physical_device,
                                         VkFormat depth_format)
        -> Vulkan::VulkanGraphicsPipeline {
        const auto vertex_artifact =
            Vulkan::loadSpirv(std::filesystem::path{SIGNUM_TEST_SHADER_DIR} /
                              "triangle.vert.spv");

        const auto fragment_artifact =
            Vulkan::loadSpirv(std::filesystem::path{SIGNUM_TEST_SHADER_DIR} /
                              "triangle.frag.spv");

        const Vulkan::GraphicsShaderData shaders{
            .vertex =
                {
                    .words = vertex_artifact.words,
                    .selection =
                        {
                            .stage = Shader::ShaderStage::Vertex,
                            .entry_point = "main",
                        },
                },
            .fragment =
                {
                    .words = fragment_artifact.words,
                    .selection =
                        {
                            .stage = Shader::ShaderStage::Fragment,
                            .entry_point = "main",
                        },
                },
        };

        constexpr std::array<VkVertexInputBindingDescription, 1> bindings{
            VkVertexInputBindingDescription{
                .binding = 0U,
                .stride = kVertexStride,
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            },
        };

        constexpr std::array<VkVertexInputAttributeDescription, 2> attributes{
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

        constexpr std::array<VkPushConstantRange, 1> push_constant_ranges{
            VkPushConstantRange{
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .offset = 0U,
                .size = static_cast<std::uint32_t>(sizeof(Math::Matrix4f)),
            },
        };

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(physical_device, &properties);

        const Vulkan::GraphicsAttachmentFormats formats{
            .color = kColorFormat,
            .depth = depth_format,
        };

        const Vulkan::GraphicsVertexInputData vertex_input{
            .bindings = bindings,
            .attributes = attributes,
        };

        const Vulkan::GraphicsPipelineLayoutData layout{
            .push_constant_ranges = push_constant_ranges,
        };

        return Vulkan::VulkanGraphicsPipeline{
            device,       formats, shaders,
            vertex_input, layout,  properties.limits.maxPushConstantsSize,
        };
    }

    class DepthGraphicsPipelineIntegrationTest : public VulkanIntegrationTest {
      protected:
        VkFormat m_DepthFormat{VK_FORMAT_UNDEFINED};

        auto SetUp() -> void override {
            VulkanIntegrationTest::SetUp();

            if (HasFatalFailure() || IsSkipped()) {
                return;
            }

            if (!m_DynamicRenderingEnabled) {
                GTEST_SKIP() << "Dynamic rendering is unsupported";
            }

            const std::optional<VkFormat> selected_depth_format =
                Vulkan::selectSupportedDepthFormat(m_PhysicalDevice);

            if (!selected_depth_format.has_value()) {
                GTEST_SKIP() << "No supported depth attachment format";
            }

            m_DepthFormat = selected_depth_format.value();

            VkFormatProperties properties{};
            vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, kColorFormat,
                                                &properties);

            if ((properties.optimalTilingFeatures &
                 VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) == 0U) {
                GTEST_SKIP() << "Test color format is unsupported";
            }
        }

        [[nodiscard]] auto createPipeline() -> Vulkan::VulkanGraphicsPipeline {
            return makeDepthPipeline(m_Device, m_PhysicalDevice, m_DepthFormat);
        }
    };
} // namespace

TEST_F(VulkanIntegrationTest, RejectsUndefinedColorAttachmentFormat) {
    constexpr Vulkan::GraphicsAttachmentFormats formats{
        .color = VK_FORMAT_UNDEFINED,
        .depth = VK_FORMAT_D32_SFLOAT,
    };

    EXPECT_DEATH(
        static_cast<void>(Vulkan::VulkanGraphicsPipeline(
            m_Device, formats, {}, {}, {}, kTestPushConstantLimit)),
        "VulkanGraphicsPipeline requires a valid color attachment format");
}

TEST_F(VulkanIntegrationTest, RejectsUndefinedDepthAttachmentFormat) {
    constexpr Vulkan::GraphicsAttachmentFormats formats{
        .color = kColorFormat,
        .depth = VK_FORMAT_UNDEFINED,
    };

    EXPECT_DEATH(
        static_cast<void>(Vulkan::VulkanGraphicsPipeline(
            m_Device, formats, {}, {}, {}, kTestPushConstantLimit)),
        "VulkanGraphicsPipeline requires a valid depth attachment format");
}

TEST_F(DepthGraphicsPipelineIntegrationTest,
       CreatesGraphicsPipelineWithDepthAttachment) {
    const Vulkan::VulkanGraphicsPipeline pipeline = createPipeline();

    EXPECT_NE(pipeline.nativeHandle(), VK_NULL_HANDLE);
    EXPECT_NE(pipeline.layoutHandle(), VK_NULL_HANDLE);
}

TEST_F(DepthGraphicsPipelineIntegrationTest,
       SelfMoveAssignmentPreservesOwnedResources) {
    Vulkan::VulkanGraphicsPipeline pipeline = createPipeline();

    const VkPipeline original_pipeline = pipeline.nativeHandle();
    const VkPipelineLayout original_layout = pipeline.layoutHandle();

    ASSERT_NE(original_pipeline, VK_NULL_HANDLE);
    ASSERT_NE(original_layout, VK_NULL_HANDLE);

    Vulkan::VulkanGraphicsPipeline &same_pipeline = pipeline;

    pipeline = std::move(same_pipeline);

    EXPECT_EQ(pipeline.nativeHandle(), original_pipeline);
    EXPECT_EQ(pipeline.layoutHandle(), original_layout);
}

TEST_F(DepthGraphicsPipelineIntegrationTest,
       MoveConstructionTransfersPipelineAndLayoutOwnership) {
    std::optional<Vulkan::VulkanGraphicsPipeline> destination;

    VkPipeline original_pipeline{VK_NULL_HANDLE};
    VkPipelineLayout original_layout{VK_NULL_HANDLE};

    {
        Vulkan::VulkanGraphicsPipeline source = createPipeline();

        original_pipeline = source.nativeHandle();
        original_layout = source.layoutHandle();

        ASSERT_NE(original_pipeline, VK_NULL_HANDLE);
        ASSERT_NE(original_layout, VK_NULL_HANDLE);

        destination.emplace(std::move(source));
    }

    ASSERT_TRUE(destination.has_value());

    EXPECT_EQ(destination->nativeHandle(), original_pipeline);
    EXPECT_EQ(destination->layoutHandle(), original_layout);
}

TEST_F(DepthGraphicsPipelineIntegrationTest,
       MoveAssignmentTransfersOwnershipAndReplacesDestination) {
    Vulkan::VulkanGraphicsPipeline destination = createPipeline();

    const VkPipeline old_destination_pipeline = destination.nativeHandle();

    const VkPipelineLayout old_destination_layout = destination.layoutHandle();

    VkPipeline source_pipeline{VK_NULL_HANDLE};
    VkPipelineLayout source_layout{VK_NULL_HANDLE};

    {
        Vulkan::VulkanGraphicsPipeline source = createPipeline();

        source_pipeline = source.nativeHandle();
        source_layout = source.layoutHandle();

        ASSERT_NE(source_pipeline, VK_NULL_HANDLE);
        ASSERT_NE(source_layout, VK_NULL_HANDLE);

        ASSERT_NE(source_pipeline, old_destination_pipeline);
        ASSERT_NE(source_layout, old_destination_layout);

        destination = std::move(source);
    }

    EXPECT_EQ(destination.nativeHandle(), source_pipeline);
    EXPECT_EQ(destination.layoutHandle(), source_layout);

    EXPECT_NE(destination.nativeHandle(), old_destination_pipeline);
    EXPECT_NE(destination.layoutHandle(), old_destination_layout);
}

TEST_F(DepthGraphicsPipelineIntegrationTest,
       MovedFromPipelineCanReceiveNewResources) {
    Vulkan::VulkanGraphicsPipeline source = createPipeline();

    Vulkan::VulkanGraphicsPipeline destination{std::move(source)};

    source = createPipeline();

    EXPECT_NE(source.nativeHandle(), VK_NULL_HANDLE);
    EXPECT_NE(source.layoutHandle(), VK_NULL_HANDLE);

    EXPECT_NE(source.nativeHandle(), destination.nativeHandle());
    EXPECT_NE(source.layoutHandle(), destination.layoutHandle());
}
