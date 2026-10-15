#include "engine/renderer/pipeline/blend_factor.hpp"
#include "engine/renderer/pipeline/blend_operation.hpp"
#include "engine/renderer/pipeline/color_blend_state.hpp"
#include "engine/renderer/pipeline/color_write_mask.hpp"
#include "engine/renderer/pipeline/compare_op.hpp"
#include "engine/renderer/pipeline/cull_mode.hpp"
#include "engine/renderer/pipeline/front_face.hpp"
#include "engine/renderer/pipeline/graphics_pipeline_state.hpp"
#include "engine/renderer/pipeline/primitive_topology.hpp"
#include <gtest/gtest.h>

namespace Renderer = SNE::Engine::Renderer;

TEST(ColorWriteMaskTests, DefaultMasksCompareEqual) {
    const Renderer::ColorWriteMask first{};
    const Renderer::ColorWriteMask second{};

    EXPECT_TRUE(first == second);
}

TEST(ColorWriteMaskTests, DetectsChangesToEveryChannel) {
    const Renderer::ColorWriteMask original{};
    Renderer::ColorWriteMask modified = original;

    modified.red = false;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.green = false;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.blue = false;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.alpha = false;
    EXPECT_FALSE(original == modified);
}

TEST(ColorBlendStateTests, DefaultStatesCompareEqual) {
    const Renderer::ColorBlendState first{};
    const Renderer::ColorBlendState second{};

    EXPECT_TRUE(first == second);
}

TEST(ColorBlendStateTests, DetectsChangesToEveryMember) {
    const Renderer::ColorBlendState original{};
    Renderer::ColorBlendState modified = original;

    modified.enabled = true;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.source_color_factor = Renderer::BlendFactor::One;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.destination_color_factor = Renderer::BlendFactor::Zero;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.color_operation = Renderer::BlendOperation::Subtract;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.source_alpha_factor = Renderer::BlendFactor::Zero;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.destination_alpha_factor = Renderer::BlendFactor::One;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.alpha_operation = Renderer::BlendOperation::Max;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.write_mask.red = false;
    EXPECT_FALSE(original == modified);
}

TEST(ColorBlendStateTests, ComparesFactorsEvenWhenBlendingIsDisabled) {
    const Renderer::ColorBlendState original{};
    Renderer::ColorBlendState modified = original;

    ASSERT_FALSE(original.enabled);
    ASSERT_FALSE(modified.enabled);

    modified.source_color_factor = Renderer::BlendFactor::One;

    EXPECT_FALSE(original == modified);
}

TEST(GraphicsPipelineStateTests, DefaultStatesCompareEqual) {
    const Renderer::GraphicsPipelineState first{};
    const Renderer::GraphicsPipelineState second{};

    EXPECT_TRUE(first == second);
}

TEST(GraphicsPipelineStateTests, DetectsChangesToDepthSettings) {
    const Renderer::GraphicsPipelineState original{};
    Renderer::GraphicsPipelineState modified = original;

    modified.depth_test_enabled = false;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.depth_write_enabled = false;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.depth_compare = Renderer::CompareOp::Greater;
    EXPECT_FALSE(original == modified);
}

TEST(GraphicsPipelineStateTests, DetectsChangesToRasterizationSettings) {
    const Renderer::GraphicsPipelineState original{};
    Renderer::GraphicsPipelineState modified = original;

    modified.cull_mode = Renderer::CullMode::Back;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.front_face = Renderer::FrontFace::Clockwise;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.topology = Renderer::PrimitiveTopology::LineList;
    EXPECT_FALSE(original == modified);
}

TEST(GraphicsPipelineStateTests, DetectsChangesToNestedBlendState) {
    const Renderer::GraphicsPipelineState original{};
    Renderer::GraphicsPipelineState modified = original;

    modified.color_blend.enabled = true;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.color_blend.source_color_factor = Renderer::BlendFactor::One;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.color_blend.write_mask.alpha = false;
    EXPECT_FALSE(original == modified);
}

TEST(GraphicsPipelineStateTests, RestoringConfigurationRestoresEquality) {
    const Renderer::GraphicsPipelineState original{};
    Renderer::GraphicsPipelineState modified = original;

    modified.cull_mode = Renderer::CullMode::Front;
    modified.depth_compare = Renderer::CompareOp::GreaterOrEqual;
    modified.color_blend.enabled = true;

    ASSERT_FALSE(original == modified);

    modified = original;

    EXPECT_TRUE(original == modified);
}
