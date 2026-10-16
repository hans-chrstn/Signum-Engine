#include "engine/renderer/texture/texture_format.hpp"
#include "engine/renderer/texture/texture_upload_data.hpp"
#include "engine/renderer/vulkan/texture/vulkan_texture_format.hpp"
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
