#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/renderer/shader/shader_entry_point_selection.hpp"
#include "engine/renderer/shader/shader_interface.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_spirv.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_spirv_reflection.hpp"
#include <array>
#include <cstdint>
#include <filesystem>
#include <gtest/gtest.h>
#include <span>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;
namespace Shader = SNE::Engine::Renderer::Shader;
namespace Core = SNE::Engine::Core;

namespace {
    [[nodiscard]] auto fixturePath(const char *filename)
        -> std::filesystem::path {
        return std::filesystem::path{SIGNUM_TEST_FIXTURE_DIR} / filename;
    }

    [[nodiscard]] auto vertexSelection() -> Shader::ShaderEntryPointSelection {
        return {
            .stage = Shader::ShaderStage::Vertex,
            .entry_point = "main",
        };
    }

    [[nodiscard]] auto
    reflectFixture(const char *filename,
                   const Shader::ShaderEntryPointSelection &selection)
        -> Shader::ShaderInterface {
        const auto artifact = Vulkan::loadSpirv(fixturePath(filename));

        return Vulkan::reflectSpirvInterface(artifact.words, selection);
    }
} // namespace

TEST(SpirvReflectionTests, PreservesSelectedVertexEntryPoint) {
    const auto result = reflectFixture("triangle.vert.spv", vertexSelection());

    EXPECT_EQ(result.entry_point.stage, Shader::ShaderStage::Vertex);
    EXPECT_EQ(result.entry_point.entry_point, "main");
}

TEST(SpirvReflectionTests, PreservesSelectedComputeEntryPoint) {
    const Shader::ShaderEntryPointSelection selection{
        .stage = Shader::ShaderStage::Compute,
        .entry_point = "main",
    };

    const auto result = reflectFixture("matrix_transform.comp.spv", selection);

    EXPECT_EQ(result.entry_point.stage, Shader::ShaderStage::Compute);
    EXPECT_EQ(result.entry_point.entry_point, "main");
}

TEST(SpirvReflectionTests, ReturnsEmptyPushConstantRangesWhenAbsent) {
    const Shader::ShaderEntryPointSelection selection{
        .stage = Shader::ShaderStage::Fragment,
        .entry_point = "main",
    };

    const auto result = reflectFixture("triangle.frag.spv", selection);

    EXPECT_EQ(result.entry_point.stage, Shader::ShaderStage::Fragment);
    EXPECT_EQ(result.entry_point.entry_point, "main");
    EXPECT_TRUE(result.push_constant_ranges.empty());
}

TEST(SpirvReflectionTests, ReflectsPushConstantOffset) {
    const auto result =
        reflectFixture("push_constant.vert.spv", vertexSelection());

    ASSERT_EQ(result.push_constant_ranges.size(), 1U);

    EXPECT_EQ(result.push_constant_ranges[0].offset, 0U);
}

TEST(SpirvReflectionTests, ReflectsPushConstantSize) {
    const auto result =
        reflectFixture("push_constant.vert.spv", vertexSelection());

    ASSERT_EQ(result.push_constant_ranges.size(), 1U);

    EXPECT_EQ(result.push_constant_ranges[0].size, 16U);
}

TEST(SpirvReflectionTests, ReflectsPushConstantStage) {
    const auto result =
        reflectFixture("push_constant.vert.spv", vertexSelection());

    ASSERT_EQ(result.push_constant_ranges.size(), 1U);

    EXPECT_EQ(result.push_constant_ranges[0].stage,
              Shader::ShaderStage::Vertex);
}

TEST(SpirvReflectionTests, ReflectsNonzeroPushConstantOffset) {
    const auto result =
        reflectFixture("push_constant_offset.vert.spv", vertexSelection());

    ASSERT_EQ(result.push_constant_ranges.size(), 1U);

    const auto &range = result.push_constant_ranges[0];

    EXPECT_EQ(range.offset, 16U);
    EXPECT_EQ(range.size, 16U);
    EXPECT_EQ(range.stage, Shader::ShaderStage::Vertex);
}

TEST(SpirvReflectionTests, PreservesReflectedMetadataAfterReturn) {
    const Shader::ShaderInterface result = [] -> Shader::ShaderInterface {
        const auto artifact =
            Vulkan::loadSpirv(fixturePath("push_constant.vert.spv"));

        return Vulkan::reflectSpirvInterface(artifact.words, vertexSelection());
    }();

    EXPECT_EQ(result.entry_point.stage, Shader::ShaderStage::Vertex);
    EXPECT_EQ(result.entry_point.entry_point, "main");

    ASSERT_EQ(result.push_constant_ranges.size(), 1U);

    EXPECT_EQ(result.push_constant_ranges[0].offset, 0U);
    EXPECT_EQ(result.push_constant_ranges[0].size, 16U);
    EXPECT_EQ(result.push_constant_ranges[0].stage,
              Shader::ShaderStage::Vertex);
}

TEST(SpirvReflectionTests, RejectsEmptyModule) {
    const std::span<const std::uint32_t> words{};

    try {
        (void)Vulkan::reflectSpirvInterface(words, vertexSelection());
        FAIL() << "Expected empty SPIR-V module to be rejected";
    } catch (const Core::Error::EngineError &error) {
        EXPECT_EQ(error.getCode(),
                  Core::Error::Code::VulkanSpirvValidationFailed);
    }
}

TEST(SpirvReflectionTests, RejectsMalformedModule) {
    // clang-format off
    const std::array<std::uint32_t, 5> words{
        0x00000000U,
        0x00010600U,
        0U,
        1U,
        0U,
    };
    // clang-format on

    try {
        (void)Vulkan::reflectSpirvInterface(words, vertexSelection());
        FAIL() << "Expected malformed SPIR-V module to be rejected";
    } catch (const Core::Error::EngineError &error) {
        EXPECT_EQ(error.getCode(),
                  Core::Error::Code::VulkanSpirvValidationFailed);
    }
}

TEST(SpirvReflectionTests, RejectsMissingEntryPoint) {
    const Shader::ShaderEntryPointSelection selection{
        .stage = Shader::ShaderStage::Vertex,
        .entry_point = "nonexistentEntryPoint",
    };

    try {
        (void)reflectFixture("triangle.vert.spv", selection);
        FAIL() << "Expected missing entry point to be rejected";
    } catch (const Core::Error::EngineError &error) {
        EXPECT_EQ(error.getCode(),
                  Core::Error::Code::VulkanShaderEntryPointUnavailable);
    }
}

TEST(SpirvReflectionTests, RejectsMismatchedEntryPointStage) {
    const Shader::ShaderEntryPointSelection selection{
        .stage = Shader::ShaderStage::Fragment,
        .entry_point = "main",
    };

    try {
        (void)reflectFixture("triangle.vert.spv", selection);
        FAIL() << "Expected mismatched shader stage to be rejected";
    } catch (const Core::Error::EngineError &error) {
        EXPECT_EQ(error.getCode(),
                  Core::Error::Code::VulkanShaderEntryPointUnavailable);
    }
}
