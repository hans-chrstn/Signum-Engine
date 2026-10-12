#include "../vulkan_integration_test_fixture.hpp"
#include "engine/renderer/vulkan/device/vulkan_depth_format.hpp"
#include <array>
#include <gtest/gtest.h>
#include <optional>
#include <vulkan/vulkan.h>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

TEST_F(VulkanIntegrationTest, SelectsSupportedDepthFormatFromPhysicalDevice) {
    ASSERT_NE(m_PhysicalDevice, VK_NULL_HANDLE);

    constexpr std::array<VkFormat, 2> preferred_formats{
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D16_UNORM,
    };

    std::optional<VkFormat> expected{};

    for (const VkFormat format : preferred_formats) {
        VkFormatProperties properties{};

        vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format,
                                            &properties);

        if ((properties.optimalTilingFeatures &
             VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0U) {
            expected = format;
            break;
        }
    }

    const std::optional<VkFormat> selected =
        Vulkan::selectSupportedDepthFormat(m_PhysicalDevice);

    EXPECT_EQ(selected, expected);
}
