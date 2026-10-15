#include "engine/renderer/shader/graphics_shader_data.hpp"
#include "engine/renderer/shader/shader_artifact.hpp"
#include "engine/renderer/shader/shader_entry_point_selection.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include "engine/renderer/shader/shader_stage_input.hpp"
#include <cstdint>
#include <gtest/gtest.h>

namespace Shader = SNE::Engine::Renderer::Shader;

namespace {
    constexpr std::uint32_t kFirstWord{1U};
    constexpr std::uint32_t kSecondWord{2U};
    constexpr std::uint32_t kDifferentWord{3U};

    [[nodiscard]] auto makeStageInput(Shader::ShaderStage stage)
        -> Shader::ShaderStageInput {
        return {
            .artifact =
                {
                    .words = {kFirstWord, kSecondWord},
                },
            .selection =
                {
                    .stage = stage,
                    .entry_point = "main",
                },
        };
    }

    [[nodiscard]] auto makeGraphicsShaderData() -> Shader::GraphicsShaderData {
        return {
            .vertex = makeStageInput(Shader::ShaderStage::Vertex),
            .fragment = makeStageInput(Shader::ShaderStage::Fragment),
        };
    }
} // namespace

TEST(ShaderArtifactTests, IdenticalWordsCompareEqual) {
    const Shader::ShaderArtifact first{
        .words = {kFirstWord, kSecondWord},
    };

    const Shader::ShaderArtifact second{
        .words = {kFirstWord, kSecondWord},
    };

    EXPECT_TRUE(first == second);
}

TEST(ShaderArtifactTests, DifferentWordsCompareUnequal) {
    const Shader::ShaderArtifact first{
        .words = {kFirstWord, kSecondWord},
    };

    const Shader::ShaderArtifact second{
        .words = {kFirstWord, kDifferentWord},
    };

    EXPECT_FALSE(first == second);
}

TEST(ShaderArtifactTests, DifferentWordCountsCompareUnequal) {
    const Shader::ShaderArtifact first{
        .words = {kFirstWord, kSecondWord},
    };

    const Shader::ShaderArtifact second{
        .words = {kFirstWord},
    };

    EXPECT_FALSE(first == second);
}

TEST(ShaderArtifactTests, EmptyArtifactsCompareEqual) {
    const Shader::ShaderArtifact first{};
    const Shader::ShaderArtifact second{};

    EXPECT_TRUE(first == second);
}

TEST(ShaderEntryPointSelectionTests, IdenticalSelectionsCompareEqual) {
    const Shader::ShaderEntryPointSelection first{
        .stage = Shader::ShaderStage::Vertex,
        .entry_point = "main",
    };

    const Shader::ShaderEntryPointSelection second{
        .stage = Shader::ShaderStage::Vertex,
        .entry_point = "main",
    };

    EXPECT_TRUE(first == second);
}

TEST(ShaderEntryPointSelectionTests, DifferentStagesCompareUnequal) {
    const Shader::ShaderEntryPointSelection first{
        .stage = Shader::ShaderStage::Vertex,
        .entry_point = "main",
    };

    const Shader::ShaderEntryPointSelection second{
        .stage = Shader::ShaderStage::Fragment,
        .entry_point = "main",
    };

    EXPECT_FALSE(first == second);
}

TEST(ShaderEntryPointSelectionTests, DifferentEntryPointsCompareUnequal) {
    const Shader::ShaderEntryPointSelection first{
        .stage = Shader::ShaderStage::Vertex,
        .entry_point = "main",
    };

    const Shader::ShaderEntryPointSelection second{
        .stage = Shader::ShaderStage::Vertex,
        .entry_point = "alternative",
    };

    EXPECT_FALSE(first == second);
}

TEST(ShaderStageInputTests, IdenticalInputsCompareEqual) {
    const Shader::ShaderStageInput first =
        makeStageInput(Shader::ShaderStage::Vertex);

    const Shader::ShaderStageInput second =
        makeStageInput(Shader::ShaderStage::Vertex);

    EXPECT_TRUE(first == second);
}

TEST(ShaderStageInputTests, DifferentArtifactsCompareUnequal) {
    const Shader::ShaderStageInput original =
        makeStageInput(Shader::ShaderStage::Vertex);

    Shader::ShaderStageInput modified = original;
    modified.artifact.words[0] = kDifferentWord;

    EXPECT_FALSE(original == modified);
}

TEST(ShaderStageInputTests, DifferentSelectionsCompareUnequal) {
    const Shader::ShaderStageInput original =
        makeStageInput(Shader::ShaderStage::Vertex);

    Shader::ShaderStageInput modified = original;
    modified.selection.entry_point = "alternative";

    EXPECT_FALSE(original == modified);
}

TEST(GraphicsShaderDataTests, IdenticalConfigurationsCompareEqual) {
    const Shader::GraphicsShaderData first = makeGraphicsShaderData();
    const Shader::GraphicsShaderData second = makeGraphicsShaderData();

    EXPECT_TRUE(first == second);
}

TEST(GraphicsShaderDataTests, DifferentVertexShadersCompareUnequal) {
    const Shader::GraphicsShaderData original = makeGraphicsShaderData();

    Shader::GraphicsShaderData modified = original;
    modified.vertex.artifact.words[0] = kDifferentWord;

    EXPECT_FALSE(original == modified);
}

TEST(GraphicsShaderDataTests, DifferentFragmentShadersCompareUnequal) {
    const Shader::GraphicsShaderData original = makeGraphicsShaderData();

    Shader::GraphicsShaderData modified = original;
    modified.fragment.artifact.words[0] = kDifferentWord;

    EXPECT_FALSE(original == modified);
}

TEST(GraphicsShaderDataTests, DifferentEntryPointsCompareUnequal) {
    const Shader::GraphicsShaderData original = makeGraphicsShaderData();

    Shader::GraphicsShaderData modified = original;
    modified.vertex.selection.entry_point = "alternative";

    EXPECT_FALSE(original == modified);
}

TEST(GraphicsShaderDataTests, RestoringConfigurationRestoresEquality) {
    const Shader::GraphicsShaderData original = makeGraphicsShaderData();

    Shader::GraphicsShaderData modified = original;
    modified.fragment.selection.entry_point = "alternative";

    ASSERT_FALSE(original == modified);

    modified = original;

    EXPECT_TRUE(original == modified);
}
