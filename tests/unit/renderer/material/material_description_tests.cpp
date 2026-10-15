#include "engine/renderer/material/material_description.hpp"
#include "engine/renderer/pipeline/cull_mode.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include <cstdint>
#include <gtest/gtest.h>

namespace Renderer = SNE::Engine::Renderer;
namespace Shader = SNE::Engine::Renderer::Shader;

namespace {
    constexpr std::uint32_t kVertexShaderWord{1U};
    constexpr std::uint32_t kFragmentShaderWord{2U};
    constexpr std::uint32_t kModifiedShaderWord{3U};

    [[nodiscard]] auto makeMaterial() -> Renderer::MaterialDescription {
        return {
            .shaders =
                {
                    .vertex =
                        {
                            .artifact = {.words = {kVertexShaderWord}},
                            .selection =
                                {
                                    .stage = Shader::ShaderStage::Vertex,
                                    .entry_point = "main",
                                },
                        },
                    .fragment =
                        {
                            .artifact = {.words = {kFragmentShaderWord}},
                            .selection =
                                {
                                    .stage = Shader::ShaderStage::Fragment,
                                    .entry_point = "main",
                                },
                        },
                },
            .pipeline_state = {},
        };
    }
} // namespace

TEST(MaterialDescriptionTests, DefaultDescriptionsCompareEqual) {
    const Renderer::MaterialDescription first{};
    const Renderer::MaterialDescription second{};

    EXPECT_TRUE(first == second);
}

TEST(MaterialDescriptionTests,
     IndependentlyConstructedDescriptionsCompareEqual) {
    const Renderer::MaterialDescription first = makeMaterial();
    const Renderer::MaterialDescription second = makeMaterial();

    EXPECT_TRUE(first == second);
}

TEST(MaterialDescriptionTests, DifferentVertexShadersCompareUnequal) {
    const Renderer::MaterialDescription original = makeMaterial();
    Renderer::MaterialDescription modified = makeMaterial();

    modified.shaders.vertex.artifact.words.front() = kModifiedShaderWord;

    EXPECT_FALSE(original == modified);
}

TEST(MaterialDescriptionTests, DifferentFragmentShadersCompareUnequal) {
    const Renderer::MaterialDescription original = makeMaterial();
    Renderer::MaterialDescription modified = makeMaterial();

    modified.shaders.fragment.artifact.words.front() = kModifiedShaderWord;

    EXPECT_FALSE(original == modified);
}

TEST(MaterialDescriptionTests, DifferentEntryPointsCompareUnequal) {
    const Renderer::MaterialDescription original = makeMaterial();
    Renderer::MaterialDescription modified = makeMaterial();

    modified.shaders.vertex.selection.entry_point = "alternate";

    EXPECT_FALSE(original == modified);

    modified = makeMaterial();
    modified.shaders.fragment.selection.entry_point = "alternate";

    EXPECT_FALSE(original == modified);
}

TEST(MaterialDescriptionTests, DifferentShaderStagesCompareUnequal) {
    const Renderer::MaterialDescription original = makeMaterial();
    Renderer::MaterialDescription modified = makeMaterial();

    modified.shaders.vertex.selection.stage = Shader::ShaderStage::Compute;

    EXPECT_FALSE(original == modified);
}

TEST(MaterialDescriptionTests, DifferentDepthStatesCompareUnequal) {
    const Renderer::MaterialDescription original = makeMaterial();
    Renderer::MaterialDescription modified = makeMaterial();

    modified.pipeline_state.depth_test_enabled = false;

    EXPECT_FALSE(original == modified);

    modified = makeMaterial();
    modified.pipeline_state.depth_write_enabled = false;

    EXPECT_FALSE(original == modified);
}

TEST(MaterialDescriptionTests, DifferentCullModesCompareUnequal) {
    const Renderer::MaterialDescription original = makeMaterial();
    Renderer::MaterialDescription modified = makeMaterial();

    modified.pipeline_state.cull_mode = Renderer::CullMode::Back;

    EXPECT_FALSE(original == modified);
}

TEST(MaterialDescriptionTests, DifferentBlendStatesCompareUnequal) {
    const Renderer::MaterialDescription original = makeMaterial();
    Renderer::MaterialDescription modified = makeMaterial();

    modified.pipeline_state.color_blend.enabled = true;

    EXPECT_FALSE(original == modified);
}

TEST(MaterialDescriptionTests, RestoringConfigurationRestoresEquality) {
    const Renderer::MaterialDescription original = makeMaterial();
    Renderer::MaterialDescription modified = makeMaterial();

    modified.pipeline_state.cull_mode = Renderer::CullMode::Back;

    EXPECT_FALSE(original == modified);

    modified.pipeline_state.cull_mode = original.pipeline_state.cull_mode;

    EXPECT_TRUE(original == modified);
}
