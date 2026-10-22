#include "engine/renderer/texture/texture_address_mode.hpp"
#include "engine/renderer/texture/texture_sampler_description.hpp"
#include "engine/renderer/vulkan/texture/vulkan_texture_address_mode.hpp"
#include <bit>
#include <cstdint>
#include <gtest/gtest.h>
#include <vulkan/vulkan.h>

namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

TEST(VulkanTextureAddressModeTests, ConvertsRepeat) {
    EXPECT_EQ(
        Vulkan::toVkSamplerAddressMode(Renderer::TextureAddressMode::Repeat),
        VK_SAMPLER_ADDRESS_MODE_REPEAT);
}

TEST(VulkanTextureAddressModeTests, ConvertsMirroredRepeat) {
    EXPECT_EQ(Vulkan::toVkSamplerAddressMode(
                  Renderer::TextureAddressMode::MirroredRepeat),
              VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT);
}

TEST(VulkanTextureAddressModeTests, ConvertsClampToEdge) {
    EXPECT_EQ(Vulkan::toVkSamplerAddressMode(
                  Renderer::TextureAddressMode::ClampToEdge),
              VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
}

TEST(VulkanTextureAddressModeTests, RejectsUnsupportedAddressMode) {
    constexpr std::uint8_t kUnsupportedAddressModeValue{255U};

    const auto invalid_mode = std::bit_cast<Renderer::TextureAddressMode>(
        kUnsupportedAddressModeValue);

    ASSERT_DEATH(
        static_cast<void>(Vulkan::toVkSamplerAddressMode(invalid_mode)),
        "Vulkan texture-address-mode conversion requires a supported "
        "TextureAddressMode value");
}

TEST(TextureSamplerDescriptionTests, DefaultsToRepeatAddressing) {
    const Renderer::TextureSamplerDescription sampler{};

    EXPECT_EQ(sampler.address_mode_u, Renderer::TextureAddressMode::Repeat);
    EXPECT_EQ(sampler.address_mode_v, Renderer::TextureAddressMode::Repeat);
    EXPECT_EQ(sampler.address_mode_w, Renderer::TextureAddressMode::Repeat);
}

TEST(TextureSamplerDescriptionTests, SupportsIndependentAddressModes) {
    const Renderer::TextureSamplerDescription sampler{
        .address_mode_u = Renderer::TextureAddressMode::Repeat,
        .address_mode_v = Renderer::TextureAddressMode::ClampToEdge,
        .address_mode_w = Renderer::TextureAddressMode::MirroredRepeat,
    };

    EXPECT_EQ(sampler.address_mode_u, Renderer::TextureAddressMode::Repeat);
    EXPECT_EQ(sampler.address_mode_v,
              Renderer::TextureAddressMode::ClampToEdge);
    EXPECT_EQ(sampler.address_mode_w,
              Renderer::TextureAddressMode::MirroredRepeat);
}
