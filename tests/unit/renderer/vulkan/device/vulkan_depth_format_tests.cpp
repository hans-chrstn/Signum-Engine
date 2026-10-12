
#include "engine/renderer/vulkan/device/vulkan_depth_format.hpp"
#include <array>
#include <gtest/gtest.h>
#include <optional>
#include <vulkan/vulkan.h>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    constexpr VkFormatFeatureFlags kDepthFeature =
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    constexpr VkFormatFeatureFlags kUnrelatedFeature =
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
} // namespace

TEST(VulkanDepthFormatTests, PrefersFirstSupportedFormat) {
    constexpr std::array<Vulkan::DepthFormatCandidate, 2> candidates{
        {
            {
                .format = VK_FORMAT_D32_SFLOAT,
                .optimal_tiling_features = kDepthFeature,
            },
            {
                .format = VK_FORMAT_D16_UNORM,
                .optimal_tiling_features = kDepthFeature,
            },
        },
    };

    const std::optional<VkFormat> selected =
        Vulkan::selectPreferredDepthFormat(candidates);

    EXPECT_EQ(selected, std::optional<VkFormat>{VK_FORMAT_D32_SFLOAT});
}

TEST(VulkanDepthFormatTests, FallsBackWhenPreferredFormatUnsupported) {
    constexpr std::array<Vulkan::DepthFormatCandidate, 2> candidates{
        {
            {
                .format = VK_FORMAT_D32_SFLOAT,
                .optimal_tiling_features = kUnrelatedFeature,
            },
            {
                .format = VK_FORMAT_D16_UNORM,
                .optimal_tiling_features = kDepthFeature,
            },
        },
    };

    const std::optional<VkFormat> selected =
        Vulkan::selectPreferredDepthFormat(candidates);

    EXPECT_EQ(selected, std::optional<VkFormat>{VK_FORMAT_D16_UNORM});
}

TEST(VulkanDepthFormatTests, ReturnsNulloptWhenNoFormatSupportsDepth) {
    constexpr std::array<Vulkan::DepthFormatCandidate, 2> candidates{
        {
            {
                .format = VK_FORMAT_D32_SFLOAT,
                .optimal_tiling_features = kUnrelatedFeature,
            },
            {
                .format = VK_FORMAT_D16_UNORM,
                .optimal_tiling_features = 0U,
            },
        },
    };

    const std::optional<VkFormat> selected =
        Vulkan::selectPreferredDepthFormat(candidates);

    EXPECT_EQ(selected, std::nullopt);
}

TEST(VulkanDepthFormatTests, ReturnsNulloptForEmptyCandidates) {
    constexpr std::array<Vulkan::DepthFormatCandidate, 0> candidates{};

    const std::optional<VkFormat> selected =
        Vulkan::selectPreferredDepthFormat(candidates);

    EXPECT_EQ(selected, std::nullopt);
}

TEST(VulkanDepthFormatTests, AcceptsDepthFeatureCombinedWithOtherFeatures) {
    constexpr std::array<Vulkan::DepthFormatCandidate, 2> candidates{
        {
            {
                .format = VK_FORMAT_D32_SFLOAT,
                .optimal_tiling_features = kDepthFeature | kUnrelatedFeature,
            },
            {
                .format = VK_FORMAT_D16_UNORM,
                .optimal_tiling_features = kDepthFeature,
            },
        },
    };

    const std::optional<VkFormat> selected =
        Vulkan::selectPreferredDepthFormat(candidates);

    EXPECT_EQ(selected, std::optional<VkFormat>{VK_FORMAT_D32_SFLOAT});
}

TEST(VulkanDepthFormatTests, RejectsNullPhysicalDeviceHandle) {
    EXPECT_DEATH(
        static_cast<void>(Vulkan::selectSupportedDepthFormat(VK_NULL_HANDLE)),
        "Depth-format selection requires a valid Vulkan physical device");
}
