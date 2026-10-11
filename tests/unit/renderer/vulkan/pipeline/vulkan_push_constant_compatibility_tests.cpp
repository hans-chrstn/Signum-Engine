#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/renderer/shader/push_constant_range.hpp"
#include "engine/renderer/shader/shader_interface.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_push_constant_compatibility.hpp"
#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <initializer_list>
#include <limits>
#include <span>
#include <vulkan/vulkan.h>

namespace Core = SNE::Engine::Core;
namespace Shader = SNE::Engine::Renderer::Shader;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    constexpr std::uint32_t kDeviceLimit = 128U;
    constexpr std::uint32_t kZeroOffset = 0U;
    constexpr std::uint32_t kZeroSize = 0U;
    constexpr std::uint32_t kBlockSize = 16U;
    constexpr std::uint32_t kExtendedBlockSize = 32U;
    constexpr std::uint32_t kNextBlockOffset = kBlockSize;
    constexpr std::uint32_t kNearLimitOffset = kDeviceLimit - kBlockSize;
    constexpr std::uint32_t kMisalignedOffset = 2U;
    constexpr std::uint32_t kMisalignedSize = 6U;
    constexpr std::uint32_t kPushConstantAlignment = 4U;

    constexpr std::uint32_t kMaximumAlignedOffset =
        std::numeric_limits<std::uint32_t>::max() -
        (kPushConstantAlignment - 1U);

    constexpr VkShaderStageFlags kVertexStage =
        static_cast<VkShaderStageFlags>(VK_SHADER_STAGE_VERTEX_BIT);

    constexpr VkShaderStageFlags kFragmentStage =
        static_cast<VkShaderStageFlags>(VK_SHADER_STAGE_FRAGMENT_BIT);

    constexpr VkShaderStageFlags kComputeStage =
        static_cast<VkShaderStageFlags>(VK_SHADER_STAGE_COMPUTE_BIT);

    constexpr VkShaderStageFlags kVertexFragmentStages =
        kVertexStage | kFragmentStage;

    [[nodiscard]] constexpr auto shaderRange(Shader::ShaderStage stage,
                                             std::uint32_t offset,
                                             std::uint32_t size)
        -> Shader::PushConstantRange {
        return {
            .offset = offset,
            .size = size,
            .stage = stage,
        };
    }

    [[nodiscard]] constexpr auto pipelineRange(std::uint32_t offset,
                                               std::uint32_t size,
                                               VkShaderStageFlags stages)
        -> VkPushConstantRange {
        return {
            .stageFlags = stages,
            .offset = offset,
            .size = size,
        };
    }

    [[nodiscard]] auto
    makeInterface(std::initializer_list<Shader::PushConstantRange> requirements)
        -> Shader::ShaderInterface {
        Shader::ShaderInterface result{};
        result.push_constant_ranges.assign(requirements);
        return result;
    }

    auto
    expectIncompatible(const Shader::ShaderInterface &shader_interface,
                       std::span<const VkPushConstantRange> pipeline_ranges)
        -> void {
        try {
            Vulkan::validatePushConstantCompatibility(
                shader_interface, pipeline_ranges, kDeviceLimit);

            FAIL() << "Expected incompatible push-constant layout";
        } catch (const Core::Error::EngineError &error) {
            EXPECT_EQ(error.getCode(),
                      Core::Error::Code::VulkanPushConstantLayoutIncompatible);
        }
    }
} // namespace

TEST(PushConstantCompatibilityTests, AcceptsEmptyShaderAndPipelineRanges) {
    const Shader::ShaderInterface shader_interface{};

    EXPECT_NO_THROW(Vulkan::validatePushConstantCompatibility(
        shader_interface, {}, kDeviceLimit));
}

TEST(PushConstantCompatibilityTests, AcceptsUnusedPipelineRange) {
    const Shader::ShaderInterface shader_interface{};

    const std::array ranges{
        pipelineRange(kZeroOffset, kBlockSize, kVertexStage),
    };

    EXPECT_NO_THROW(Vulkan::validatePushConstantCompatibility(
        shader_interface, ranges, kDeviceLimit));
}

TEST(PushConstantCompatibilityTests, AcceptsExactVertexRange) {
    const auto shader_interface = makeInterface({
        shaderRange(Shader::ShaderStage::Vertex, kZeroOffset, kBlockSize),
    });

    const std::array ranges{
        pipelineRange(kZeroOffset, kBlockSize, kVertexStage),
    };

    EXPECT_NO_THROW(Vulkan::validatePushConstantCompatibility(
        shader_interface, ranges, kDeviceLimit));
}

TEST(PushConstantCompatibilityTests, AcceptsExactFragmentRange) {
    const auto shader_interface = makeInterface({
        shaderRange(Shader::ShaderStage::Fragment, kZeroOffset, kBlockSize),
    });

    const std::array ranges{
        pipelineRange(kZeroOffset, kBlockSize, kFragmentStage),
    };

    EXPECT_NO_THROW(Vulkan::validatePushConstantCompatibility(
        shader_interface, ranges, kDeviceLimit));
}

TEST(PushConstantCompatibilityTests, AcceptsExactComputeRange) {
    const auto shader_interface = makeInterface({
        shaderRange(Shader::ShaderStage::Compute, kZeroOffset, kBlockSize),
    });

    const std::array ranges{
        pipelineRange(kZeroOffset, kBlockSize, kComputeStage),
    };

    EXPECT_NO_THROW(Vulkan::validatePushConstantCompatibility(
        shader_interface, ranges, kDeviceLimit));
}

TEST(PushConstantCompatibilityTests, AcceptsPipelineRangeCoveringSubrange) {
    const auto shader_interface = makeInterface({
        shaderRange(Shader::ShaderStage::Vertex, kNextBlockOffset, kBlockSize),
    });

    const std::array ranges{
        pipelineRange(kZeroOffset, kExtendedBlockSize, kVertexStage),
    };

    EXPECT_NO_THROW(Vulkan::validatePushConstantCompatibility(
        shader_interface, ranges, kDeviceLimit));
}

TEST(PushConstantCompatibilityTests, AcceptsCombinedShaderStages) {
    const auto shader_interface = makeInterface({
        shaderRange(Shader::ShaderStage::Vertex, kZeroOffset, kBlockSize),
        shaderRange(Shader::ShaderStage::Fragment, kZeroOffset, kBlockSize),
    });

    const std::array ranges{
        pipelineRange(kZeroOffset, kBlockSize, kVertexFragmentStages),
    };

    EXPECT_NO_THROW(Vulkan::validatePushConstantCompatibility(
        shader_interface, ranges, kDeviceLimit));
}

TEST(PushConstantCompatibilityTests, AcceptsOverlappingDifferentStages) {
    const auto shader_interface = makeInterface({
        shaderRange(Shader::ShaderStage::Vertex, kZeroOffset, kBlockSize),
        shaderRange(Shader::ShaderStage::Fragment, kZeroOffset, kBlockSize),
    });

    const std::array ranges{
        pipelineRange(kZeroOffset, kExtendedBlockSize, kVertexStage),
        pipelineRange(kZeroOffset, kBlockSize, kFragmentStage),
    };

    EXPECT_NO_THROW(Vulkan::validatePushConstantCompatibility(
        shader_interface, ranges, kDeviceLimit));
}

TEST(PushConstantCompatibilityTests, AcceptsDisjointDifferentStages) {
    const auto shader_interface = makeInterface({
        shaderRange(Shader::ShaderStage::Vertex, kZeroOffset, kBlockSize),
        shaderRange(Shader::ShaderStage::Fragment, kNextBlockOffset,
                    kBlockSize),
    });

    const std::array ranges{
        pipelineRange(kZeroOffset, kBlockSize, kVertexStage),
        pipelineRange(kNextBlockOffset, kBlockSize, kFragmentStage),
    };

    EXPECT_NO_THROW(Vulkan::validatePushConstantCompatibility(
        shader_interface, ranges, kDeviceLimit));
}

TEST(PushConstantCompatibilityTests, PreservesEarlierSuccessfulMatch) {
    const auto shader_interface = makeInterface({
        shaderRange(Shader::ShaderStage::Vertex, kZeroOffset, kBlockSize),
    });

    const std::array ranges{
        pipelineRange(kZeroOffset, kBlockSize, kVertexStage),
        pipelineRange(kZeroOffset, kBlockSize, kFragmentStage),
    };

    EXPECT_NO_THROW(Vulkan::validatePushConstantCompatibility(
        shader_interface, ranges, kDeviceLimit));
}

TEST(PushConstantCompatibilityTests, FindsCompatibleRangeAfterMismatch) {
    const auto shader_interface = makeInterface({
        shaderRange(Shader::ShaderStage::Vertex, kZeroOffset, kBlockSize),
    });

    const std::array ranges{
        pipelineRange(kZeroOffset, kBlockSize, kFragmentStage),
        pipelineRange(kZeroOffset, kBlockSize, kVertexStage),
    };

    EXPECT_NO_THROW(Vulkan::validatePushConstantCompatibility(
        shader_interface, ranges, kDeviceLimit));
}

TEST(PushConstantCompatibilityTests, AcceptsRangeEndingAtDeviceLimit) {
    const auto shader_interface = makeInterface({
        shaderRange(Shader::ShaderStage::Vertex, kNearLimitOffset, kBlockSize),
    });

    const std::array ranges{
        pipelineRange(kNearLimitOffset, kBlockSize, kVertexStage),
    };

    EXPECT_NO_THROW(Vulkan::validatePushConstantCompatibility(
        shader_interface, ranges, kDeviceLimit));
}

TEST(PushConstantCompatibilityTests, RejectsMissingPipelineRanges) {
    const auto shader_interface = makeInterface({
        shaderRange(Shader::ShaderStage::Vertex, kZeroOffset, kBlockSize),
    });

    expectIncompatible(shader_interface, {});
}

TEST(PushConstantCompatibilityTests, RejectsMissingShaderStage) {
    const auto shader_interface = makeInterface({
        shaderRange(Shader::ShaderStage::Vertex, kZeroOffset, kBlockSize),
    });

    const std::array ranges{
        pipelineRange(kZeroOffset, kBlockSize, kFragmentStage),
    };

    expectIncompatible(shader_interface, ranges);
}

TEST(PushConstantCompatibilityTests, RejectsPipelineStartingTooLate) {
    const auto shader_interface = makeInterface({
        shaderRange(Shader::ShaderStage::Vertex, kZeroOffset, kBlockSize),
    });

    const std::array ranges{
        pipelineRange(kNextBlockOffset, kBlockSize, kVertexStage),
    };

    expectIncompatible(shader_interface, ranges);
}

TEST(PushConstantCompatibilityTests, RejectsPipelineEndingTooEarly) {
    const auto shader_interface = makeInterface({
        shaderRange(Shader::ShaderStage::Vertex, kZeroOffset,
                    kExtendedBlockSize),
    });

    const std::array ranges{
        pipelineRange(kZeroOffset, kBlockSize, kVertexStage),
    };

    expectIncompatible(shader_interface, ranges);
}

TEST(PushConstantCompatibilityTests, RejectsIncompleteMultipleRequirements) {
    const auto shader_interface = makeInterface({
        shaderRange(Shader::ShaderStage::Vertex, kZeroOffset, kBlockSize),
        shaderRange(Shader::ShaderStage::Fragment, kZeroOffset, kBlockSize),
    });

    const std::array ranges{
        pipelineRange(kZeroOffset, kBlockSize, kVertexStage),
    };

    expectIncompatible(shader_interface, ranges);
}

TEST(PushConstantCompatibilityTests, RejectsZeroDeviceLimit) {
    const Shader::ShaderInterface shader_interface{};

    ASSERT_DEATH(Vulkan::validatePushConstantCompatibility(shader_interface, {},
                                                           kZeroSize),
                 "nonzero device limit");
}

TEST(PushConstantCompatibilityTests, RejectsZeroPipelineRangeSize) {
    const Shader::ShaderInterface shader_interface{};

    const std::array ranges{
        pipelineRange(kZeroOffset, kZeroSize, kVertexStage),
    };

    ASSERT_DEATH(Vulkan::validatePushConstantCompatibility(
                     shader_interface, ranges, kDeviceLimit),
                 "nonzero size");
}

TEST(PushConstantCompatibilityTests, RejectsMisalignedPipelineOffset) {
    const Shader::ShaderInterface shader_interface{};

    const std::array ranges{
        pipelineRange(kMisalignedOffset, kBlockSize, kVertexStage),
    };

    ASSERT_DEATH(Vulkan::validatePushConstantCompatibility(
                     shader_interface, ranges, kDeviceLimit),
                 "offset must be aligned");
}

TEST(PushConstantCompatibilityTests, RejectsMisalignedPipelineSize) {
    const Shader::ShaderInterface shader_interface{};

    const std::array ranges{
        pipelineRange(kZeroOffset, kMisalignedSize, kVertexStage),
    };

    ASSERT_DEATH(Vulkan::validatePushConstantCompatibility(
                     shader_interface, ranges, kDeviceLimit),
                 "size must be aligned");
}

TEST(PushConstantCompatibilityTests, RejectsEmptyPipelineStageFlags) {
    const Shader::ShaderInterface shader_interface{};

    const std::array ranges{
        pipelineRange(kZeroOffset, kBlockSize, VkShaderStageFlags{}),
    };

    ASSERT_DEATH(Vulkan::validatePushConstantCompatibility(
                     shader_interface, ranges, kDeviceLimit),
                 "at least one shader stage");
}

TEST(PushConstantCompatibilityTests, RejectsDuplicateShaderStages) {
    const Shader::ShaderInterface shader_interface{};

    const std::array ranges{
        pipelineRange(kZeroOffset, kBlockSize, kVertexStage),
        pipelineRange(kNextBlockOffset, kBlockSize, kVertexStage),
    };

    ASSERT_DEATH(Vulkan::validatePushConstantCompatibility(
                     shader_interface, ranges, kDeviceLimit),
                 "must not repeat shader");
}

TEST(PushConstantCompatibilityTests, RejectsDuplicateCombinedStageUsage) {
    const Shader::ShaderInterface shader_interface{};

    const std::array ranges{
        pipelineRange(kZeroOffset, kBlockSize, kVertexFragmentStages),
        pipelineRange(kNextBlockOffset, kBlockSize, kFragmentStage),
    };

    ASSERT_DEATH(Vulkan::validatePushConstantCompatibility(
                     shader_interface, ranges, kDeviceLimit),
                 "must not repeat shader");
}

TEST(PushConstantCompatibilityTests, RejectsPipelineExceedingDeviceLimit) {
    const Shader::ShaderInterface shader_interface{};

    const std::array ranges{
        pipelineRange(kNearLimitOffset, kExtendedBlockSize, kVertexStage),
    };

    ASSERT_DEATH(Vulkan::validatePushConstantCompatibility(
                     shader_interface, ranges, kDeviceLimit),
                 "exceeds the physical device limit");
}

TEST(PushConstantCompatibilityTests, RejectsHugePipelineOffset) {
    const Shader::ShaderInterface shader_interface{};

    const std::array ranges{
        pipelineRange(kMaximumAlignedOffset, kBlockSize, kVertexStage),
    };

    ASSERT_DEATH(Vulkan::validatePushConstantCompatibility(
                     shader_interface, ranges, kDeviceLimit),
                 "exceeds the physical device limit");
}

TEST(PushConstantCompatibilityTests, RejectsZeroShaderRequirementSize) {
    const auto shader_interface = makeInterface({
        shaderRange(Shader::ShaderStage::Vertex, kZeroOffset, kZeroSize),
    });

    ASSERT_DEATH(Vulkan::validatePushConstantCompatibility(shader_interface, {},
                                                           kDeviceLimit),
                 "nonzero size");
}

TEST(PushConstantCompatibilityTests, RejectsMisalignedShaderOffset) {
    const auto shader_interface = makeInterface({
        shaderRange(Shader::ShaderStage::Vertex, kMisalignedOffset, kBlockSize),
    });

    ASSERT_DEATH(Vulkan::validatePushConstantCompatibility(shader_interface, {},
                                                           kDeviceLimit),
                 "aligned to 4 bytes");
}

TEST(PushConstantCompatibilityTests, RejectsMisalignedShaderSize) {
    const auto shader_interface = makeInterface({
        shaderRange(Shader::ShaderStage::Vertex, kZeroOffset, kMisalignedSize),
    });

    ASSERT_DEATH(Vulkan::validatePushConstantCompatibility(shader_interface, {},
                                                           kDeviceLimit),
                 "aligned to 4 bytes");
}

TEST(PushConstantCompatibilityTests, RejectsShaderExceedingDeviceLimit) {
    const auto shader_interface = makeInterface({
        shaderRange(Shader::ShaderStage::Vertex, kNearLimitOffset,
                    kExtendedBlockSize),
    });

    ASSERT_DEATH(Vulkan::validatePushConstantCompatibility(shader_interface, {},
                                                           kDeviceLimit),
                 "exceeds the physical device limit");
}

TEST(PushConstantCompatibilityTests, RejectsHugeShaderOffset) {
    const auto shader_interface = makeInterface({
        shaderRange(Shader::ShaderStage::Vertex, kMaximumAlignedOffset,
                    kBlockSize),
    });

    ASSERT_DEATH(Vulkan::validatePushConstantCompatibility(shader_interface, {},
                                                           kDeviceLimit),
                 "exceeds the physical device limit");
}
