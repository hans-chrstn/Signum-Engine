#include "engine/renderer/lighting/directional_light_uniform_data.hpp"
#include <gtest/gtest.h>
#include <limits>

namespace Renderer = SNE::Engine::Renderer;

TEST(DirectionalLightUniformDataTests, NormalizesAndPacksLightProperties) {
    const Renderer::DirectionalLight light{
        .direction_to_light = {.x = 0.0F, .y = 3.0F, .z = 4.0F},
        .color = {.x = 0.8F, .y = 0.6F, .z = 0.4F},
        .intensity = 2.0F,
    };

    const Renderer::DirectionalLightUniformData data =
        Renderer::makeDirectionalLightUniformData(light);

    EXPECT_FLOAT_EQ(data.direction.x, 0.0F);
    EXPECT_FLOAT_EQ(data.direction.y, 0.6F);
    EXPECT_FLOAT_EQ(data.direction.z, 0.8F);
    EXPECT_FLOAT_EQ(data.direction.w, 0.0F);

    EXPECT_FLOAT_EQ(data.color_intensity.x, 0.8F);
    EXPECT_FLOAT_EQ(data.color_intensity.y, 0.6F);
    EXPECT_FLOAT_EQ(data.color_intensity.z, 0.4F);
    EXPECT_FLOAT_EQ(data.color_intensity.w, 2.0F);
}

TEST(DirectionalLightUniformDataTests, PreservesNegativeDirectionComponents) {
    const Renderer::DirectionalLight light{
        .direction_to_light = {.x = -3.0F, .y = 0.0F, .z = 4.0F},
        .color = {.x = 1.0F, .y = 1.0F, .z = 1.0F},
        .intensity = 1.0F,
    };

    const auto data = Renderer::makeDirectionalLightUniformData(light);

    EXPECT_FLOAT_EQ(data.direction.x, -0.6F);
    EXPECT_FLOAT_EQ(data.direction.y, 0.0F);
    EXPECT_FLOAT_EQ(data.direction.z, 0.8F);
    EXPECT_FLOAT_EQ(data.direction.w, 0.0F);
}

TEST(DirectionalLightUniformDataTests, NormalizesVeryLargeDirection) {
    const float maximum = std::numeric_limits<float>::max();

    const Renderer::DirectionalLight light{
        .direction_to_light = {.x = maximum},
        .color = {.x = 1.0F, .y = 1.0F, .z = 1.0F},
        .intensity = 1.0F,
    };

    const auto data = Renderer::makeDirectionalLightUniformData(light);

    EXPECT_FLOAT_EQ(data.direction.x, 1.0F);
    EXPECT_FLOAT_EQ(data.direction.y, 0.0F);
    EXPECT_FLOAT_EQ(data.direction.z, 0.0F);
}

TEST(DirectionalLightUniformDataTests, NormalizesVerySmallDirection) {
    const float minimum = std::numeric_limits<float>::min();

    const Renderer::DirectionalLight light{
        .direction_to_light = {.y = minimum},
        .color = {.x = 1.0F, .y = 1.0F, .z = 1.0F},
        .intensity = 1.0F,
    };

    const auto data = Renderer::makeDirectionalLightUniformData(light);

    EXPECT_FLOAT_EQ(data.direction.x, 0.0F);
    EXPECT_FLOAT_EQ(data.direction.y, 1.0F);
    EXPECT_FLOAT_EQ(data.direction.z, 0.0F);
}

TEST(DirectionalLightUniformDataTests, PreservesHdrColorAndZeroIntensity) {
    const Renderer::DirectionalLight light{
        .direction_to_light = {.z = 1.0F},
        .color = {.x = 2.0F, .y = 4.0F, .z = 8.0F},
        .intensity = 0.0F,
    };

    const auto data = Renderer::makeDirectionalLightUniformData(light);

    EXPECT_FLOAT_EQ(data.color_intensity.x, 2.0F);
    EXPECT_FLOAT_EQ(data.color_intensity.y, 4.0F);
    EXPECT_FLOAT_EQ(data.color_intensity.z, 8.0F);
    EXPECT_FLOAT_EQ(data.color_intensity.w, 0.0F);
}

TEST(DirectionalLightUniformDataTests, RejectsInvalidDirection) {
    const Renderer::DirectionalLight light{
        .direction_to_light = {},
        .color = {.x = 1.0F, .y = 1.0F, .z = 1.0F},
        .intensity = 1.0F,
    };

    ASSERT_DEATH(
        static_cast<void>(Renderer::makeDirectionalLightUniformData(light)),
        "DirectionalLight requires a finite, nonzero direction");
}
