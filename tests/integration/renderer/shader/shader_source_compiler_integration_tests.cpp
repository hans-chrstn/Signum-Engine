#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/renderer/shader/shader_source_compiler.hpp"
#include "engine/renderer/shader/shader_source_description.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_spirv_validation.hpp"
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

namespace Core = SNE::Engine::Core;
namespace Shader = SNE::Engine::Renderer::Shader;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    constexpr std::uint32_t kSpirvMagicNumber{0x07230203U};

    [[nodiscard]] auto fixturePath(std::string_view filename)
        -> std::filesystem::path {

        return std::filesystem::path{SIGNUM_TEST_FIXTURE_DIR} /
               std::string{filename};
    }

    [[nodiscard]] auto makeShaderSource(Shader::ShaderStage stage,
                                        std::string entry_point)
        -> Shader::ShaderSourceDescription {

        return {
            .path = fixturePath("source_compiler_valid.slang"),
            .language = Shader::ShaderSourceLanguage::Slang,
            .stage = stage,
            .entry_point = std::move(entry_point),
        };
    }

    auto expectCompilationFailure(const Shader::ShaderSourceDescription &source,
                                  Core::Error::Code expected_code,
                                  std::string_view expected_operation) -> void {

        try {
            static_cast<void>(Shader::compileShaderSource(source));
            FAIL() << "Expected shader source compilation to fail";
        } catch (const Core::Error::EngineError &error) {
            EXPECT_EQ(error.getCode(), expected_code);
            EXPECT_EQ(error.getOperation(), expected_operation);
            EXPECT_FALSE(std::string_view{error.what()}.empty());
        }
    }

    auto expectValidCompilation(Shader::ShaderStage stage,
                                const std::string &entry_point) -> void {

        const Shader::ShaderSourceDescription source =
            makeShaderSource(stage, entry_point);

        const Shader::ShaderCompilationResult result =
            Shader::compileShaderSource(source);

        EXPECT_EQ(result.stage_input.selection.stage, stage);
        EXPECT_EQ(result.stage_input.selection.entry_point, entry_point);

        const auto &words = result.stage_input.artifact.words;

        ASSERT_FALSE(words.empty());
        EXPECT_EQ(words.front(), kSpirvMagicNumber);

        EXPECT_NO_THROW(Vulkan::validateSpirvModule(words));

        EXPECT_NO_THROW(Vulkan::validateSpirvEntryPoint(
            words, result.stage_input.selection));
    }
} // namespace

TEST(ShaderSourceCompilerIntegrationTests, CompilesVertexShader) {
    expectValidCompilation(Shader::ShaderStage::Vertex, "vertexMain");
}

TEST(ShaderSourceCompilerIntegrationTests, CompilesFragmentShader) {
    expectValidCompilation(Shader::ShaderStage::Fragment, "fragmentMain");
}

TEST(ShaderSourceCompilerIntegrationTests, CompilesComputeShader) {
    expectValidCompilation(Shader::ShaderStage::Compute, "computeMain");
}

TEST(ShaderSourceCompilerIntegrationTests, RejectsEmptySourcePath) {
    Shader::ShaderSourceDescription source =
        makeShaderSource(Shader::ShaderStage::Fragment, "fragmentMain");

    source.path.clear();

    expectCompilationFailure(source, Core::Error::Code::ShaderSourceInvalid,
                             "Validate Shader Source");
}

TEST(ShaderSourceCompilerIntegrationTests, RejectsEmptyEntryPoint) {
    Shader::ShaderSourceDescription source =
        makeShaderSource(Shader::ShaderStage::Fragment, "fragmentMain");

    source.entry_point.clear();

    expectCompilationFailure(source, Core::Error::Code::ShaderSourceInvalid,
                             "Validate Shader Source");
}

TEST(ShaderSourceCompilerIntegrationTests, RejectsUnsupportedSourceLanguage) {
    Shader::ShaderSourceDescription source =
        makeShaderSource(Shader::ShaderStage::Fragment, "fragmentMain");

    source.language = Shader::ShaderSourceLanguage::Hlsl;

    expectCompilationFailure(source, Core::Error::Code::ShaderSourceInvalid,
                             "Validate Shader Source Language");
}

TEST(ShaderSourceCompilerIntegrationTests, RejectsMissingSourceFile) {
    Shader::ShaderSourceDescription source =
        makeShaderSource(Shader::ShaderStage::Fragment, "fragmentMain");

    source.path = fixturePath("source_compiler_nonexistent.slang");

    ASSERT_FALSE(std::filesystem::exists(source.path));

    expectCompilationFailure(source, Core::Error::Code::ShaderSourceLoadFailed,
                             "Load Shader Source");
}

TEST(ShaderSourceCompilerIntegrationTests, RejectsMissingEntryPoint) {
    const Shader::ShaderSourceDescription source =
        makeShaderSource(Shader::ShaderStage::Fragment, "doesNotExist");

    expectCompilationFailure(source,
                             Core::Error::Code::ShaderSourceCompilationFailed,
                             "Find Slang Shader Entry Point");
}

TEST(ShaderSourceCompilerIntegrationTests, RejectsMismatchedShaderStage) {
    const Shader::ShaderSourceDescription source =
        makeShaderSource(Shader::ShaderStage::Vertex, "fragmentMain");

    expectCompilationFailure(source, Core::Error::Code::ShaderSourceInvalid,
                             "Validate Shader Stage");
}

TEST(ShaderSourceCompilerIntegrationTests, ReportsMalformedSourceDiagnostics) {
    Shader::ShaderSourceDescription source =
        makeShaderSource(Shader::ShaderStage::Fragment, "fragmentMain");

    source.path = fixturePath("source_compiler_invalid.slang");

    try {
        static_cast<void>(Shader::compileShaderSource(source));
        FAIL() << "Expected malformed Slang shader to fail";
    } catch (const Core::Error::EngineError &error) {
        EXPECT_EQ(error.getCode(),
                  Core::Error::Code::ShaderSourceCompilationFailed);

        const std::string_view message{error.what()};

        EXPECT_FALSE(message.empty());

        // The message should contain compiler diagnostics, not only
        // the engine's generic failure description.
        EXPECT_NE(message.find('\n'), std::string_view::npos);
    }
}

TEST(ShaderSourceCompilerIntegrationTests, CompilesSameShaderRepeatedly) {
    const Shader::ShaderSourceDescription source =
        makeShaderSource(Shader::ShaderStage::Fragment, "fragmentMain");

    const Shader::ShaderCompilationResult first =
        Shader::compileShaderSource(source);

    const Shader::ShaderCompilationResult second =
        Shader::compileShaderSource(source);

    ASSERT_FALSE(first.stage_input.artifact.words.empty());
    ASSERT_FALSE(second.stage_input.artifact.words.empty());

    EXPECT_EQ(first.stage_input.artifact, second.stage_input.artifact);

    EXPECT_EQ(first.stage_input.selection, second.stage_input.selection);
}

TEST(ShaderSourceCompilerIntegrationTests,
     RemainsUsableAfterCompilationFailure) {
    Shader::ShaderSourceDescription invalid_source =
        makeShaderSource(Shader::ShaderStage::Fragment, "fragmentMain");

    invalid_source.path = fixturePath("source_compiler_invalid.slang");

    EXPECT_THROW(static_cast<void>(Shader::compileShaderSource(invalid_source)),
                 Core::Error::EngineError);

    const Shader::ShaderSourceDescription valid_source =
        makeShaderSource(Shader::ShaderStage::Fragment, "fragmentMain");

    const Shader::ShaderCompilationResult result =
        Shader::compileShaderSource(valid_source);

    ASSERT_FALSE(result.stage_input.artifact.words.empty());

    EXPECT_NO_THROW(Vulkan::validateSpirvEntryPoint(
        result.stage_input.artifact.words, result.stage_input.selection));
}

TEST(ShaderSourceCompilerIntegrationTests,
     RejectsEntryPointContainingEmbeddedNull) {

    Shader::ShaderSourceDescription source =
        makeShaderSource(Shader::ShaderStage::Fragment, "fragmentMain");

    source.entry_point.push_back('\0');
    source.entry_point.append("unexpected");

    expectCompilationFailure(source, Core::Error::Code::ShaderSourceInvalid,
                             "Validate Shader Source");
}

TEST(ShaderSourceCompilerIntegrationTests,
     RejectsSourceFileContainingEmbeddedNull) {

    const auto unique_id =
        std::chrono::steady_clock::now().time_since_epoch().count();

    const std::filesystem::path shader_path =
        std::filesystem::temp_directory_path() /
        ("signum_shader_embedded_null_" + std::to_string(unique_id) + ".slang");

    struct ScopedShaderFile {
        std::filesystem::path path;

        ~ScopedShaderFile() {
            std::error_code ignored_error;
            static_cast<void>(std::filesystem::remove(path, ignored_error));
        }
    };

    const ScopedShaderFile temporary_file{shader_path};

    std::string source_text{"[shader(\"fragment\")]\n"};
    source_text.push_back('\0');
    source_text.append("float4 fragmentMain() : SV_Target\n"
                       "{\n"
                       "    return float4(1.0, 0.0, 0.0, 1.0);\n"
                       "}\n");

    {
        std::ofstream file{
            temporary_file.path,
            std::ios::binary | std::ios::trunc,
        };

        ASSERT_TRUE(file.is_open());

        file.write(source_text.data(),
                   static_cast<std::streamsize>(source_text.size()));

        ASSERT_TRUE(file.good());

        file.close();

        ASSERT_FALSE(file.fail());
    }

    Shader::ShaderSourceDescription source =
        makeShaderSource(Shader::ShaderStage::Fragment, "fragmentMain");

    source.path = temporary_file.path;

    expectCompilationFailure(source, Core::Error::Code::ShaderSourceInvalid,
                             "Validate Shader Source Text");
}
