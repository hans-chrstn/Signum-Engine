#include "engine/renderer/lighting/directional_light.hpp"
#include <gtest/gtest.h>
#include <limits>

namespace Renderer = SNE::Engine::Renderer;

namespace {
    constexpr float kValidDirectionY{2.0F};
    constexpr float kValidDirectionZ{3.0F};

    constexpr float kHdrColorRed{2.0F};
    constexpr float kHdrColorGreen{4.0F};
    constexpr float kHdrColorBlue{8.0F};
    constexpr float kHdrIntensity{10.0F};

    constexpr float kNegativeColorComponent{-0.1F};

    [[nodiscard]] auto makeValidDirectionalLight()
        -> Renderer::DirectionalLight {
        return Renderer::DirectionalLight{
            .direction_to_light =
                {
                    .x = 1.0F,
                    .y = kValidDirectionY,
                    .z = kValidDirectionZ,
                },
            .color = {.x = 1.0F, .y = 1.0F, .z = 1.0F},
            .intensity = 1.0F,
        };
    }
} // namespace

TEST(DirectionalLightTests, AcceptsValidDirectionalLight) {
    const Renderer::DirectionalLight light = makeValidDirectionalLight();

    Renderer::validateDirectionalLight(light);
}

TEST(DirectionalLightTests, AcceptsZeroIntensityAndBlackColor) {
    Renderer::DirectionalLight light = makeValidDirectionalLight();

    light.color = {};
    light.intensity = 0.0F;

    Renderer::validateDirectionalLight(light);
}

TEST(DirectionalLightTests, AcceptsHdrColorAndIntensity) {
    Renderer::DirectionalLight light = makeValidDirectionalLight();

    light.color = {
        .x = kHdrColorRed,
        .y = kHdrColorGreen,
        .z = kHdrColorBlue,
    };
    light.intensity = kHdrIntensity;

    Renderer::validateDirectionalLight(light);
}

TEST(DirectionalLightTests, AcceptsAxisAlignedDirections) {
    Renderer::DirectionalLight light = makeValidDirectionalLight();

    light.direction_to_light = {.x = 1.0F};
    Renderer::validateDirectionalLight(light);

    light.direction_to_light = {.y = 1.0F};
    Renderer::validateDirectionalLight(light);

    light.direction_to_light = {.z = 1.0F};
    Renderer::validateDirectionalLight(light);
}

TEST(DirectionalLightTests, RejectsZeroDirection) {
    Renderer::DirectionalLight light = makeValidDirectionalLight();

    light.direction_to_light = {};

    ASSERT_DEATH(Renderer::validateDirectionalLight(light),
                 "DirectionalLight requires a finite, nonzero direction");
}

TEST(DirectionalLightTests, RejectsNonFiniteDirectionComponents) {
    const float nan = std::numeric_limits<float>::quiet_NaN();
    const float infinity = std::numeric_limits<float>::infinity();

    Renderer::DirectionalLight light = makeValidDirectionalLight();

    light.direction_to_light.x = nan;
    ASSERT_DEATH(Renderer::validateDirectionalLight(light),
                 "DirectionalLight requires a finite, nonzero direction");

    light = makeValidDirectionalLight();
    light.direction_to_light.y = infinity;
    ASSERT_DEATH(Renderer::validateDirectionalLight(light),
                 "DirectionalLight requires a finite, nonzero direction");

    light = makeValidDirectionalLight();
    light.direction_to_light.z = -infinity;
    ASSERT_DEATH(Renderer::validateDirectionalLight(light),
                 "DirectionalLight requires a finite, nonzero direction");
}

TEST(DirectionalLightTests, RejectsNegativeColorComponents) {
    Renderer::DirectionalLight light = makeValidDirectionalLight();

    light.color.x = kNegativeColorComponent;
    ASSERT_DEATH(
        Renderer::validateDirectionalLight(light),
        "DirectionalLight requires finite, nonnegative color components");

    light = makeValidDirectionalLight();
    light.color.y = kNegativeColorComponent;
    ASSERT_DEATH(
        Renderer::validateDirectionalLight(light),
        "DirectionalLight requires finite, nonnegative color components");

    light = makeValidDirectionalLight();
    light.color.z = kNegativeColorComponent;
    ASSERT_DEATH(
        Renderer::validateDirectionalLight(light),
        "DirectionalLight requires finite, nonnegative color components");
}

TEST(DirectionalLightTests, RejectsNonFiniteColorComponents) {
    const float nan = std::numeric_limits<float>::quiet_NaN();
    const float infinity = std::numeric_limits<float>::infinity();

    Renderer::DirectionalLight light = makeValidDirectionalLight();

    light.color.x = nan;
    ASSERT_DEATH(
        Renderer::validateDirectionalLight(light),
        "DirectionalLight requires finite, nonnegative color components");

    light = makeValidDirectionalLight();
    light.color.y = infinity;
    ASSERT_DEATH(
        Renderer::validateDirectionalLight(light),
        "DirectionalLight requires finite, nonnegative color components");

    light = makeValidDirectionalLight();
    light.color.z = -infinity;
    ASSERT_DEATH(
        Renderer::validateDirectionalLight(light),
        "DirectionalLight requires finite, nonnegative color components");
}

TEST(DirectionalLightTests, RejectsNegativeIntensity) {
    Renderer::DirectionalLight light = makeValidDirectionalLight();

    light.intensity = -1.0F;

    ASSERT_DEATH(Renderer::validateDirectionalLight(light),
                 "DirectionalLight requires finite, nonnegative intensity");
}

TEST(DirectionalLightTests, RejectsNonFiniteIntensity) {
    const float nan = std::numeric_limits<float>::quiet_NaN();
    const float infinity = std::numeric_limits<float>::infinity();

    Renderer::DirectionalLight light = makeValidDirectionalLight();

    light.intensity = nan;
    ASSERT_DEATH(Renderer::validateDirectionalLight(light),
                 "DirectionalLight requires finite, nonnegative intensity");

    light.intensity = infinity;
    ASSERT_DEATH(Renderer::validateDirectionalLight(light),
                 "DirectionalLight requires finite, nonnegative intensity");

    light.intensity = -infinity;
    ASSERT_DEATH(Renderer::validateDirectionalLight(light),
                 "DirectionalLight requires finite, nonnegative intensity");
}
