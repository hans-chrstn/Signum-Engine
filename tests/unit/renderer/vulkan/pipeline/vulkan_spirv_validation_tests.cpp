#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/renderer/shader/shader_entry_point_selection.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_spirv.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_spirv_validation.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <span>
#include <string_view>
#include <system_error>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;
namespace Core = SNE::Engine::Core;
namespace Shader = SNE::Engine::Renderer::Shader;

namespace {
    struct ScopedShaderTestFile {
        std::filesystem::path path;

        ~ScopedShaderTestFile() {
            std::error_code ignored_error;
            std::filesystem::remove(path, ignored_error);
        }
    };
} // namespace

TEST(SpirvValidationTests, RejectsEmptyModule) {
    const std::span<const std::uint32_t> words{};

    EXPECT_THROW(Vulkan::validateSpirvModule(words), Core::Error::EngineError);
}

TEST(SpirvValidationTests, RejectsMalformedModule) {
    const std::array<std::uint32_t, 5> words{0x00000000, 0x00010600, 0, 1, 0};

    EXPECT_THROW(Vulkan::validateSpirvModule(words), Core::Error::EngineError);
}

TEST(SpirvValidationTests, AcceptsValidModule) {
    const std::filesystem::path shader_path =
        std::filesystem::path{SIGNUM_TEST_FIXTURE_DIR} / "triangle.vert.spv";

    const auto artifact = Vulkan::loadSpirv(shader_path);

    EXPECT_NO_THROW(Vulkan::validateSpirvModule(artifact.words));
}

TEST(SpirvValidationTests, ReportsValidationFailureCode) {
    const std::span<const std::uint32_t> words{};

    try {
        Vulkan::validateSpirvModule(words);
        FAIL() << "Expected SPIR-V validation to fail";
    } catch (const Core::Error::EngineError &error) {
        EXPECT_EQ(error.getCode(),
                  Core::Error::Code::VulkanSpirvValidationFailed);
    }
}

TEST(SpirvValidationTests, ProvidesNonemptyDiagnostic) {
    const std::span<const std::uint32_t> words{};

    try {
        Vulkan::validateSpirvModule(words);
        FAIL() << "Expected SPIR-V validation to fail";
    } catch (const Core::Error::EngineError &error) {
        EXPECT_FALSE(std::string_view{error.what()}.empty());
    }
}

TEST(SpirvValidationTests, AcceptsMatchingVertexEntryPoint) {
    const std::filesystem::path shader_path =
        std::filesystem::path{SIGNUM_TEST_FIXTURE_DIR} / "triangle.vert.spv";

    const auto artifact = Vulkan::loadSpirv(shader_path);

    const Shader::ShaderEntryPointSelection selection{
        .stage = Shader::ShaderStage::Vertex,
        .entry_point = "main",
    };

    EXPECT_NO_THROW(Vulkan::validateSpirvEntryPoint(artifact.words, selection));
}

TEST(SpirvValidationTests, AcceptsMatchingFragmentEntryPoint) {
    const std::filesystem::path shader_path =
        std::filesystem::path{SIGNUM_TEST_FIXTURE_DIR} / "triangle.frag.spv";

    const auto artifact = Vulkan::loadSpirv(shader_path);

    const Shader::ShaderEntryPointSelection selection{
        .stage = Shader::ShaderStage::Fragment,
        .entry_point = "main",
    };

    EXPECT_NO_THROW(Vulkan::validateSpirvEntryPoint(artifact.words, selection));
}

TEST(SpirvValidationTests, AcceptsMatchingComputeEntryPoint) {
    const std::filesystem::path shader_path =
        std::filesystem::path{SIGNUM_TEST_FIXTURE_DIR} /
        "matrix_transform.comp.spv";

    const auto artifact = Vulkan::loadSpirv(shader_path);

    const Shader::ShaderEntryPointSelection selection{
        .stage = Shader::ShaderStage::Compute,
        .entry_point = "main",
    };

    EXPECT_NO_THROW(Vulkan::validateSpirvEntryPoint(artifact.words, selection));
}

TEST(SpirvValidationTests, RejectsMissingEntryPointName) {
    const std::filesystem::path shader_path =
        std::filesystem::path{SIGNUM_TEST_FIXTURE_DIR} / "triangle.vert.spv";

    const auto artifact = Vulkan::loadSpirv(shader_path);

    const Shader::ShaderEntryPointSelection selection{
        .stage = Shader::ShaderStage::Vertex,
        .entry_point = "nonexistentEntryPoint",
    };

    EXPECT_THROW(Vulkan::validateSpirvEntryPoint(artifact.words, selection),
                 Core::Error::EngineError);
}

TEST(SpirvValidationTests, RejectsMismatchedEntryPointStage) {
    const std::filesystem::path shader_path =
        std::filesystem::path{SIGNUM_TEST_FIXTURE_DIR} / "triangle.vert.spv";

    const auto artifact = Vulkan::loadSpirv(shader_path);

    const Shader::ShaderEntryPointSelection selection{
        .stage = Shader::ShaderStage::Fragment,
        .entry_point = "main",
    };

    EXPECT_THROW(Vulkan::validateSpirvEntryPoint(artifact.words, selection),
                 Core::Error::EngineError);
}

TEST(SpirvValidationTests, ReportsEntryPointUnavailableCode) {
    const std::filesystem::path shader_path =
        std::filesystem::path{SIGNUM_TEST_FIXTURE_DIR} / "triangle.vert.spv";

    const auto artifact = Vulkan::loadSpirv(shader_path);

    const Shader::ShaderEntryPointSelection selection{
        .stage = Shader::ShaderStage::Vertex,
        .entry_point = "nonexistentEntryPoint",
    };

    try {
        Vulkan::validateSpirvEntryPoint(artifact.words, selection);
        FAIL() << "Expected entry-point validation to fail";
    } catch (const Core::Error::EngineError &error) {
        EXPECT_EQ(error.getCode(),
                  Core::Error::Code::VulkanShaderEntryPointUnavailable);
    }
}

TEST(SpirvValidationTests, RejectsMalformedModuleDuringEntryPointValidation) {
    // clang-format off
    const std::array<std::uint32_t, 5> words{
        0x00000000U,
        0x00010600U,
        0U,
        1U,
        0U,
    };
    // clang-format on

    const Shader::ShaderEntryPointSelection selection{
        .stage = Shader::ShaderStage::Vertex,
        .entry_point = "main",
    };

    try {
        Vulkan::validateSpirvEntryPoint(words, selection);
        FAIL() << "Expected malformed SPIR-V to be rejected";
    } catch (const Core::Error::EngineError &error) {
        EXPECT_EQ(error.getCode(),
                  Core::Error::Code::VulkanSpirvValidationFailed);
    }
}

TEST(SpirvArtifactLoadingTests, RejectsMissingShaderFile) {
    const std::filesystem::path missing_path =
        std::filesystem::path{SIGNUM_TEST_FIXTURE_DIR} /
        "missing_shader_artifact_for_loading_test.spv";

    ASSERT_FALSE(std::filesystem::exists(missing_path));

    try {
        static_cast<void>(Vulkan::loadSpirv(missing_path));
        FAIL() << "Expected missing shader artifact to be rejected";
    } catch (const Core::Error::EngineError &error) {
        EXPECT_EQ(error.getCode(),
                  Core::Error::Code::VulkanShaderBytecodeLoadFailed);

        EXPECT_EQ(error.getOperation(), "Open SPIR-V Shader Bytecode");
    }
}

TEST(SpirvArtifactLoadingTests, RejectsEmptyShaderFile) {
    const ScopedShaderTestFile file{
        .path = std::filesystem::path{::testing::TempDir()} /
                "signum_empty_shader_loading_test.spv",
    };

    {
        std::ofstream output{
            file.path,
            std::ios::binary | std::ios::trunc,
        };

        ASSERT_TRUE(output.is_open());

        output.close();

        ASSERT_TRUE(output.good());
    }

    try {
        static_cast<void>(Vulkan::loadSpirv(file.path));
        FAIL() << "Expected empty shader artifact to be rejected";
    } catch (const Core::Error::EngineError &error) {
        EXPECT_EQ(error.getCode(),
                  Core::Error::Code::VulkanShaderBytecodeLoadFailed);

        EXPECT_EQ(error.getOperation(), "Validate SPIR-V Shader Bytecode Size");
    }
}

TEST(SpirvArtifactLoadingTests, RejectsUnalignedShaderByteCount) {
    constexpr std::size_t kIncompleteWordByteCount{3U};

    const std::array<char, kIncompleteWordByteCount> bytes{
        'S',
        'P',
        'V',
    };

    const ScopedShaderTestFile file{
        .path = std::filesystem::path{::testing::TempDir()} /
                "signum_unaligned_shader_loading_test.spv",
    };

    {
        std::ofstream output{
            file.path,
            std::ios::binary | std::ios::trunc,
        };

        ASSERT_TRUE(output.is_open());

        output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));

        output.close();

        ASSERT_TRUE(output.good());
    }

    try {
        static_cast<void>(Vulkan::loadSpirv(file.path));
        FAIL() << "Expected unaligned shader byte count to be rejected";
    } catch (const Core::Error::EngineError &error) {
        EXPECT_EQ(error.getCode(),
                  Core::Error::Code::VulkanShaderBytecodeLoadFailed);

        EXPECT_EQ(error.getOperation(), "Validate SPIR-V Shader Bytecode Size");
    }
}
