#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/renderer/shader/shader_entry_point_selection.hpp"
#include "engine/renderer/shader/shader_interface.hpp"
#include "engine/renderer/shader/shader_resource_type.hpp"
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

    [[nodiscard]] auto
    findDescriptorBinding(const Shader::ShaderInterface &shader_interface,
                          std::uint32_t set, std::uint32_t binding)
        -> const Shader::ShaderDescriptorBinding * {
        for (const auto &candidate : shader_interface.descriptor_bindings) {
            if (candidate.set == set && candidate.binding == binding) {
                return &candidate;
            }
        }

        return nullptr;
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

TEST(SpirvReflectionTests, ReturnsEmptyDescriptorBindingsWhenAbsent) {
    const auto result = reflectFixture("triangle.vert.spv", vertexSelection());

    EXPECT_TRUE(result.descriptor_bindings.empty());
}

TEST(SpirvReflectionTests, ReflectsCombinedImageSamplerBinding) {
    const Shader::ShaderEntryPointSelection selection{
        .stage = Shader::ShaderStage::Fragment,
        .entry_point = "main",
    };

    const auto result = reflectFixture("textured.frag.spv", selection);

    ASSERT_EQ(result.descriptor_bindings.size(), 1U);

    const auto &binding = result.descriptor_bindings[0];

    EXPECT_EQ(binding.set, 0U);
    EXPECT_EQ(binding.binding, 0U);
    EXPECT_EQ(binding.type, Shader::ShaderResourceType::CombinedImageSampler);
    EXPECT_EQ(binding.count, 1U);
}

TEST(SpirvReflectionTests, ReflectsDescriptorTypesAndArrayCounts) {
    const Shader::ShaderEntryPointSelection selection{
        .stage = Shader::ShaderStage::Fragment,
        .entry_point = "main",
    };

    const auto result = reflectFixture("descriptor_types.frag.spv", selection);

    ASSERT_EQ(result.descriptor_bindings.size(), 3U);

    const auto *textures = findDescriptorBinding(result, 0U, 0U);
    ASSERT_NE(textures, nullptr);

    EXPECT_EQ(textures->type, Shader::ShaderResourceType::CombinedImageSampler);
    EXPECT_EQ(textures->count, 3U);

    const auto *uniform_buffer = findDescriptorBinding(result, 1U, 2U);
    ASSERT_NE(uniform_buffer, nullptr);

    EXPECT_EQ(uniform_buffer->type, Shader::ShaderResourceType::UniformBuffer);
    EXPECT_EQ(uniform_buffer->count, 1U);

    const auto *storage_buffer = findDescriptorBinding(result, 2U, 4U);
    ASSERT_NE(storage_buffer, nullptr);

    EXPECT_EQ(storage_buffer->type, Shader::ShaderResourceType::StorageBuffer);
    EXPECT_EQ(storage_buffer->count, 1U);
}

TEST(SpirvReflectionTests, PreservesDescriptorMetadataAfterReturn) {
    const Shader::ShaderInterface result = [] -> Shader::ShaderInterface {
        const auto artifact =
            Vulkan::loadSpirv(fixturePath("textured.frag.spv"));

        const Shader::ShaderEntryPointSelection selection{
            .stage = Shader::ShaderStage::Fragment,
            .entry_point = "main",
        };

        return Vulkan::reflectSpirvInterface(artifact.words, selection);
    }();

    ASSERT_EQ(result.descriptor_bindings.size(), 1U);

    const auto &binding = result.descriptor_bindings[0];

    EXPECT_EQ(binding.set, 0U);
    EXPECT_EQ(binding.binding, 0U);
    EXPECT_EQ(binding.type, Shader::ShaderResourceType::CombinedImageSampler);
    EXPECT_EQ(binding.count, 1U);
}

TEST(SpirvReflectionTests, RejectsUnsupportedDescriptorType) {
    const Shader::ShaderEntryPointSelection selection{
        .stage = Shader::ShaderStage::Fragment,
        .entry_point = "main",
    };

    try {
        (void)reflectFixture("unsupported_descriptor.frag.spv", selection);

        FAIL() << "Expected unsupported descriptor type to be rejected";
    } catch (const Core::Error::EngineError &error) {
        EXPECT_EQ(error.getCode(),
                  Core::Error::Code::VulkanSpirvReflectionFailed);
    }
}
