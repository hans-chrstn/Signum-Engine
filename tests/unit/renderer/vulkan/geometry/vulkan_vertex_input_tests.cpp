#include "engine/renderer/geometry/vertex_attribute_description.hpp"
#include "engine/renderer/geometry/vertex_format.hpp"
#include "engine/renderer/geometry/vertex_layout_description.hpp"
#include "engine/renderer/vulkan/geometry/vulkan_vertex_input.hpp"
#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <vulkan/vulkan.h>

namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    constexpr std::uint32_t kBindingIndex = 0U;
    constexpr std::uint32_t kPositionLocation = 0U;
    constexpr std::uint32_t kColorLocation = 1U;
    constexpr std::uint32_t kPositionOffset = 0U;
    constexpr std::uint32_t kPositionSize = 8U;
    constexpr std::uint32_t kColorSize = 12U;
    constexpr std::uint32_t kColorOffset = kPositionSize;
    constexpr std::uint32_t kVertexStride = kPositionSize + kColorSize;
    constexpr std::uint32_t kOutOfBoundsOffset = kPositionSize * 2U;

    constexpr Renderer::VertexAttributeDescription kPositionAttribute{
        .location = kPositionLocation,
        .format = Renderer::VertexFormat::Float32x2,
        .offset = kPositionOffset,
    };

    constexpr Renderer::VertexAttributeDescription kColorAttribute{
        .location = kColorLocation,
        .format = Renderer::VertexFormat::Float32x3,
        .offset = kColorOffset,
    };
} // namespace

TEST(VulkanVertexInputTests, AcceptsEmptyLayoutWithZeroStride) {
    const Renderer::VertexLayoutDescription layout{};

    const auto result = Vulkan::makeVulkanVertexInputDescriptions(layout, 0U);

    EXPECT_EQ(result.binding.binding, kBindingIndex);
    EXPECT_EQ(result.binding.stride, 0U);
    EXPECT_EQ(result.binding.inputRate, VK_VERTEX_INPUT_RATE_VERTEX);
    EXPECT_TRUE(result.attributes.empty());
}

TEST(VulkanVertexInputTests, TranslatesInterleavedVertexAttributes) {
    constexpr std::array attributes{
        kPositionAttribute,
        kColorAttribute,
    };

    const Renderer::VertexLayoutDescription layout{
        .attributes = attributes,
    };

    const auto result =
        Vulkan::makeVulkanVertexInputDescriptions(layout, kVertexStride);

    EXPECT_EQ(result.binding.binding, kBindingIndex);
    EXPECT_EQ(result.binding.stride, kVertexStride);
    EXPECT_EQ(result.binding.inputRate, VK_VERTEX_INPUT_RATE_VERTEX);

    ASSERT_EQ(result.attributes.size(), attributes.size());

    const auto &position = result.attributes[0];
    EXPECT_EQ(position.location, kPositionLocation);
    EXPECT_EQ(position.binding, kBindingIndex);
    EXPECT_EQ(position.format, VK_FORMAT_R32G32_SFLOAT);
    EXPECT_EQ(position.offset, kPositionOffset);

    const auto &color = result.attributes[1];
    EXPECT_EQ(color.location, kColorLocation);
    EXPECT_EQ(color.binding, kBindingIndex);
    EXPECT_EQ(color.format, VK_FORMAT_R32G32B32_SFLOAT);
    EXPECT_EQ(color.offset, kColorOffset);
}

TEST(VulkanVertexInputTests, AcceptsAttributeEndingExactlyAtStride) {
    constexpr std::array attributes{
        kColorAttribute,
    };

    const Renderer::VertexLayoutDescription layout{
        .attributes = attributes,
    };

    const auto result =
        Vulkan::makeVulkanVertexInputDescriptions(layout, kVertexStride);

    ASSERT_EQ(result.attributes.size(), attributes.size());
    EXPECT_EQ(result.attributes.front().offset, kColorOffset);
    EXPECT_EQ(result.attributes.front().format, VK_FORMAT_R32G32B32_SFLOAT);
}

TEST(VulkanVertexInputTests, RejectsZeroStrideWhenAttributesExist) {
    constexpr std::array attributes{
        kPositionAttribute,
    };

    const Renderer::VertexLayoutDescription layout{
        .attributes = attributes,
    };

    ASSERT_DEATH(
        static_cast<void>(
            Vulkan::makeVulkanVertexInputDescriptions(layout, 0U)),
        "Vulkan vertex input requires a nonzero stride when attributes "
        "are present");
}

TEST(VulkanVertexInputTests, RejectsAttributeLargerThanStride) {
    constexpr std::array attributes{
        kColorAttribute,
    };

    const Renderer::VertexLayoutDescription layout{
        .attributes = attributes,
    };

    ASSERT_DEATH(static_cast<void>(Vulkan::makeVulkanVertexInputDescriptions(
                     layout, kPositionSize)),
                 "Vulkan vertex attribute size exceeds vertex stride");
}

TEST(VulkanVertexInputTests, RejectsAttributeExtendingBeyondStride) {
    constexpr std::array attributes{
        Renderer::VertexAttributeDescription{
            .location = kColorLocation,
            .format = Renderer::VertexFormat::Float32x3,
            .offset = kOutOfBoundsOffset,
        },
    };

    const Renderer::VertexLayoutDescription layout{
        .attributes = attributes,
    };

    ASSERT_DEATH(static_cast<void>(Vulkan::makeVulkanVertexInputDescriptions(
                     layout, kVertexStride)),
                 "Vulkan vertex attribute extends beyond vertex stride");
}

TEST(VulkanVertexInputTests, RejectsDuplicateAttributeLocations) {
    constexpr std::array attributes{
        kPositionAttribute,
        Renderer::VertexAttributeDescription{
            .location = kPositionLocation,
            .format = Renderer::VertexFormat::Float32x3,
            .offset = kColorOffset,
        },
    };

    const Renderer::VertexLayoutDescription layout{
        .attributes = attributes,
    };

    ASSERT_DEATH(static_cast<void>(Vulkan::makeVulkanVertexInputDescriptions(
                     layout, kVertexStride)),
                 "Vulkan vertex input requires unique attribute locations");
}

TEST(VulkanVertexInputTests, OwnsTranslatedDescriptions) {
    Vulkan::VulkanVertexInputDescriptions result{};

    {
        constexpr std::array attributes{
            kPositionAttribute,
        };

        const Renderer::VertexLayoutDescription layout{
            .attributes = attributes,
        };

        result =
            Vulkan::makeVulkanVertexInputDescriptions(layout, kVertexStride);
    }

    ASSERT_EQ(result.attributes.size(), 1U);
    EXPECT_EQ(result.attributes.front().location, kPositionLocation);
    EXPECT_EQ(result.attributes.front().format, VK_FORMAT_R32G32_SFLOAT);
    EXPECT_EQ(result.attributes.front().offset, kPositionOffset);
}
