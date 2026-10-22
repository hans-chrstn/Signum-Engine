#include "engine/renderer/texture/texture_format.hpp"
#include "engine/renderer/texture/texture_upload_data.hpp"
#include "engine/renderer/vulkan/texture/vulkan_texture_format.hpp"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>
#include <vulkan/vulkan.h>

namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

TEST(VulkanTextureFormatTests, ConvertsRgba8Unorm) {
    EXPECT_EQ(Vulkan::toVkFormat(Renderer::TextureFormat::Rgba8Unorm),
              VK_FORMAT_R8G8B8A8_UNORM);
}

TEST(VulkanTextureFormatTests, ConvertsRgba8Srgb) {
    EXPECT_EQ(Vulkan::toVkFormat(Renderer::TextureFormat::Rgba8Srgb),
              VK_FORMAT_R8G8B8A8_SRGB);
}

TEST(VulkanTextureFormatTests, TextureUploadDefaultsToRgba8Unorm) {
    const Renderer::TextureUploadData upload{};

    EXPECT_EQ(upload.format, Renderer::TextureFormat::Rgba8Unorm);
}

TEST(TextureUploadDataTests, BorrowsPixelStorage) {
    constexpr std::size_t kPixelByteCount{16U};
    constexpr std::uint32_t kTextureDimension{2U};
    constexpr std::byte kUpdatedPixelValue{0xFF};

    std::array<std::byte, kPixelByteCount> pixels{};

    const Renderer::TextureUploadData upload{
        .width = kTextureDimension,
        .height = kTextureDimension,
        .pixels = pixels,
    };

    EXPECT_EQ(upload.width, kTextureDimension);
    EXPECT_EQ(upload.height, kTextureDimension);
    EXPECT_EQ(upload.pixels.data(), pixels.data());
    EXPECT_EQ(upload.pixels.size_bytes(), pixels.size());

    pixels[0] = kUpdatedPixelValue;

    EXPECT_EQ(upload.pixels[0], kUpdatedPixelValue);
}

TEST(TextureUploadDataTests, PreservesExplicitFormatAndMipLevels) {
    constexpr std::size_t kCompleteMipChainBytes{20U};
    constexpr std::uint32_t kTextureDimension{2U};
    constexpr std::uint32_t kMipLevelCount{2U};

    const std::array<std::byte, kCompleteMipChainBytes> pixels{};

    const Renderer::TextureUploadData upload{
        .width = kTextureDimension,
        .height = kTextureDimension,
        .pixels = pixels,
        .format = Renderer::TextureFormat::Rgba8Srgb,
        .mip_levels = kMipLevelCount,
    };

    EXPECT_EQ(upload.width, kTextureDimension);
    EXPECT_EQ(upload.height, kTextureDimension);
    EXPECT_EQ(upload.format, Renderer::TextureFormat::Rgba8Srgb);
    EXPECT_EQ(upload.mip_levels, kMipLevelCount);

    EXPECT_EQ(upload.pixels.data(), pixels.data());
    EXPECT_EQ(upload.pixels.size_bytes(), kCompleteMipChainBytes);
}

TEST(TextureUploadDataTests, DefaultDescriptionHasNoPixelStorage) {
    const Renderer::TextureUploadData upload{};

    EXPECT_EQ(upload.width, 0U);
    EXPECT_EQ(upload.height, 0U);
    EXPECT_TRUE(upload.pixels.empty());

    EXPECT_EQ(upload.format, Renderer::TextureFormat::Rgba8Unorm);
    EXPECT_EQ(upload.mip_levels, 1U);
}

TEST(TextureUploadDataTests,
     IdenticalPixelBytesSupportDifferentColorInterpretations) {
    constexpr std::size_t kPixelByteCount{4U};
    constexpr std::byte kChannelValue{0x80};

    const std::array<std::byte, kPixelByteCount> pixels{
        kChannelValue,
        kChannelValue,
        kChannelValue,
        kChannelValue,
    };

    const Renderer::TextureUploadData linear_upload{
        .width = 1U,
        .height = 1U,
        .pixels = pixels,
        .format = Renderer::TextureFormat::Rgba8Unorm,
    };

    const Renderer::TextureUploadData srgb_upload{
        .width = 1U,
        .height = 1U,
        .pixels = pixels,
        .format = Renderer::TextureFormat::Rgba8Srgb,
    };

    EXPECT_EQ(linear_upload.pixels.data(), srgb_upload.pixels.data());
    EXPECT_EQ(linear_upload.pixels.size_bytes(),
              srgb_upload.pixels.size_bytes());

    EXPECT_NE(linear_upload.format, srgb_upload.format);

    EXPECT_EQ(Vulkan::toVkFormat(linear_upload.format),
              VK_FORMAT_R8G8B8A8_UNORM);

    EXPECT_EQ(Vulkan::toVkFormat(srgb_upload.format), VK_FORMAT_R8G8B8A8_SRGB);

    EXPECT_NE(Vulkan::toVkFormat(linear_upload.format),
              Vulkan::toVkFormat(srgb_upload.format));
}

TEST(VulkanTextureFormatTests, RejectsUnsupportedTextureFormat) {
    constexpr std::uint8_t kUnsupportedFormatValue{255U};

    const auto invalid_format =
        std::bit_cast<Renderer::TextureFormat>(kUnsupportedFormatValue);

    ASSERT_DEATH(static_cast<void>(Vulkan::toVkFormat(invalid_format)),
                 "Vulkan texture-format conversion requires a supported "
                 "TextureFormat value");
}

TEST(TextureUploadDataTests, DefaultsToLinearSamplerFiltering) {
    const Renderer::TextureUploadData upload{};

    EXPECT_EQ(upload.sampler.magnification_filter,
              Renderer::TextureFilter::Linear);

    EXPECT_EQ(upload.sampler.minification_filter,
              Renderer::TextureFilter::Linear);
}

TEST(TextureUploadDataTests, PreservesExplicitSamplerFiltering) {
    const Renderer::TextureUploadData upload{
        .pixels = {},
        .sampler =
            {
                .magnification_filter = Renderer::TextureFilter::Nearest,
                .minification_filter = Renderer::TextureFilter::Linear,
            },
    };

    EXPECT_EQ(upload.sampler.magnification_filter,
              Renderer::TextureFilter::Nearest);

    EXPECT_EQ(upload.sampler.minification_filter,
              Renderer::TextureFilter::Linear);
}
