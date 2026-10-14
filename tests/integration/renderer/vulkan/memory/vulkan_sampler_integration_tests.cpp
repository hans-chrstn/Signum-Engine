
#include "../vulkan_integration_test_fixture.hpp"
#include "engine/renderer/vulkan/memory/vulkan_sampler.hpp"
#include <gtest/gtest.h>
#include <type_traits>
#include <utility>
#include <vulkan/vulkan.h>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    [[nodiscard]] auto makeSamplerCreateInfo() -> VkSamplerCreateInfo {
        VkSamplerCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        create_info.magFilter = VK_FILTER_LINEAR;
        create_info.minFilter = VK_FILTER_LINEAR;
        create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        create_info.maxAnisotropy = 1.0F;
        create_info.minLod = 0.0F;
        create_info.maxLod = 0.0F;

        return create_info;
    }

    static_assert(!std::is_copy_constructible_v<Vulkan::VulkanSampler>);
    static_assert(!std::is_copy_assignable_v<Vulkan::VulkanSampler>);
    static_assert(std::is_nothrow_move_constructible_v<Vulkan::VulkanSampler>);
    static_assert(std::is_nothrow_move_assignable_v<Vulkan::VulkanSampler>);
} // namespace

TEST_F(VulkanIntegrationTest, CreatesAndDestroysVulkanSampler) {
    const VkSamplerCreateInfo create_info = makeSamplerCreateInfo();

    const Vulkan::VulkanSampler sampler{m_Device, create_info};

    EXPECT_NE(sampler.nativeHandle(), VK_NULL_HANDLE);
}

TEST_F(VulkanIntegrationTest, MoveConstructsVulkanSampler) {
    const VkSamplerCreateInfo create_info = makeSamplerCreateInfo();

    Vulkan::VulkanSampler source{m_Device, create_info};

    const VkSampler original_handle = source.nativeHandle();

    ASSERT_NE(original_handle, VK_NULL_HANDLE);

    const Vulkan::VulkanSampler destination{std::move(source)};

    EXPECT_EQ(destination.nativeHandle(), original_handle);
}

TEST_F(VulkanIntegrationTest, MoveAssignsVulkanSampler) {
    const VkSamplerCreateInfo create_info = makeSamplerCreateInfo();

    Vulkan::VulkanSampler source{m_Device, create_info};
    Vulkan::VulkanSampler destination{m_Device, create_info};

    const VkSampler source_handle = source.nativeHandle();
    const VkSampler destination_handle = destination.nativeHandle();

    ASSERT_NE(source_handle, VK_NULL_HANDLE);
    ASSERT_NE(destination_handle, VK_NULL_HANDLE);
    ASSERT_NE(source_handle, destination_handle);

    destination = std::move(source);

    EXPECT_EQ(destination.nativeHandle(), source_handle);
    EXPECT_NE(destination.nativeHandle(), destination_handle);
}

TEST(VulkanSamplerPreconditionTests, RejectsNullDevice) {
    const VkSamplerCreateInfo create_info = makeSamplerCreateInfo();

    ASSERT_DEATH(
        static_cast<void>(Vulkan::VulkanSampler{VK_NULL_HANDLE, create_info}),
        "VulkanSampler requires a valid Vulkan device");
}

TEST_F(VulkanIntegrationTest, RejectsInvalidSamplerCreateInfoType) {
    VkSamplerCreateInfo create_info = makeSamplerCreateInfo();
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

    ASSERT_DEATH(
        static_cast<void>(Vulkan::VulkanSampler{m_Device, create_info}),
        "VulkanSampler requires a valid sampler creation structure type");
}
