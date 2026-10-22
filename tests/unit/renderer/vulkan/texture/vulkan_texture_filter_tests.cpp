#include "engine/renderer/texture/texture_filter.hpp"
#include "engine/renderer/texture/texture_sampler_description.hpp"
#include "engine/renderer/vulkan/texture/vulkan_texture_filter.hpp"
#include <bit>
#include <cstdint>
#include <gtest/gtest.h>
#include <vulkan/vulkan.h>

namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

TEST(VulkanTextureFilterTests, ConvertsNearestFilter) {
    EXPECT_EQ(Vulkan::toVkFilter(Renderer::TextureFilter::Nearest),
              VK_FILTER_NEAREST);
}

TEST(VulkanTextureFilterTests, ConvertsLinearFilter) {
    EXPECT_EQ(Vulkan::toVkFilter(Renderer::TextureFilter::Linear),
              VK_FILTER_LINEAR);
}

TEST(VulkanTextureFilterTests, RejectsUnsupportedFilter) {
    constexpr std::uint8_t kUnsupportedFilterValue{255U};

    const auto invalid_filter =
        std::bit_cast<Renderer::TextureFilter>(kUnsupportedFilterValue);

    ASSERT_DEATH(static_cast<void>(Vulkan::toVkFilter(invalid_filter)),
                 "Vulkan texture-filter conversion requires a supported "
                 "TextureFilter value");
}

TEST(TextureSamplerDescriptionTests, DefaultsToLinearFiltering) {
    const Renderer::TextureSamplerDescription sampler{};

    EXPECT_EQ(sampler.magnification_filter, Renderer::TextureFilter::Linear);
    EXPECT_EQ(sampler.minification_filter, Renderer::TextureFilter::Linear);
}

TEST(TextureSamplerDescriptionTests, SupportsIndependentFilterSelection) {
    const Renderer::TextureSamplerDescription sampler{
        .magnification_filter = Renderer::TextureFilter::Nearest,
        .minification_filter = Renderer::TextureFilter::Linear,
    };

    EXPECT_EQ(sampler.magnification_filter, Renderer::TextureFilter::Nearest);
    EXPECT_EQ(sampler.minification_filter, Renderer::TextureFilter::Linear);
}
