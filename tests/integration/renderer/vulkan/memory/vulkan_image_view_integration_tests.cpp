#include "../vulkan_integration_test_fixture.hpp"
#include "engine/renderer/vulkan/device/vulkan_depth_format.hpp"
#include "engine/renderer/vulkan/memory/vulkan_image.hpp"
#include "engine/renderer/vulkan/memory/vulkan_image_view.hpp"
#include <cstdint>
#include <gtest/gtest.h>
#include <type_traits>
#include <utility>
#include <vulkan/vulkan.h>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    constexpr std::uint32_t kImageWidth = 64U;
    constexpr std::uint32_t kImageHeight = 64U;
    constexpr VkFormat kColorFormat = VK_FORMAT_R8G8B8A8_UNORM;

    [[nodiscard]] auto makeImageCreateInfo(VkFormat format,
                                           VkImageUsageFlags usage)
        -> Vulkan::VulkanImageCreateInfo {
        return {
            .extent =
                {
                    .width = kImageWidth,
                    .height = kImageHeight,
                    .depth = 1U,
                },
            .image_type = VK_IMAGE_TYPE_2D,
            .format = format,
            .usage = usage,
        };
    }

    [[nodiscard]] auto makeImageViewCreateInfo(VkImage image, VkFormat format,
                                               VkImageAspectFlags aspect)
        -> VkImageViewCreateInfo {
        VkImageViewCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = image;
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = format;
        create_info.subresourceRange.aspectMask = aspect;
        create_info.subresourceRange.baseMipLevel = 0U;
        create_info.subresourceRange.levelCount = 1U;
        create_info.subresourceRange.baseArrayLayer = 0U;
        create_info.subresourceRange.layerCount = 1U;
        return create_info;
    }

    static_assert(!std::is_copy_constructible_v<Vulkan::VulkanImageView>);
    static_assert(!std::is_copy_assignable_v<Vulkan::VulkanImageView>);
    static_assert(
        std::is_nothrow_move_constructible_v<Vulkan::VulkanImageView>);
    static_assert(std::is_nothrow_move_assignable_v<Vulkan::VulkanImageView>);
} // namespace

TEST_F(VulkanIntegrationTest, CreatesAndDestroysColorImageView) {
    const Vulkan::VulkanImage image{
        memoryAllocator(),
        makeImageCreateInfo(kColorFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT),
    };

    const VkImageViewCreateInfo create_info = makeImageViewCreateInfo(
        image.nativeHandle(), kColorFormat, VK_IMAGE_ASPECT_COLOR_BIT);

    const Vulkan::VulkanImageView image_view{m_Device, create_info};

    EXPECT_NE(image_view.nativeHandle(), VK_NULL_HANDLE);
}

TEST_F(VulkanIntegrationTest, CreatesDepthImageView) {
    const auto selected_format =
        Vulkan::selectSupportedDepthFormat(m_PhysicalDevice);

    if (!selected_format.has_value()) {
        GTEST_SKIP() << "No supported depth format available";
    }

    const VkFormat depth_format = selected_format.value_or(VK_FORMAT_UNDEFINED);

    const Vulkan::VulkanImage image{
        memoryAllocator(),
        makeImageCreateInfo(depth_format,
                            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT),
    };

    const VkImageViewCreateInfo create_info = makeImageViewCreateInfo(
        image.nativeHandle(), depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);

    const Vulkan::VulkanImageView image_view{m_Device, create_info};

    EXPECT_NE(image_view.nativeHandle(), VK_NULL_HANDLE);
}

TEST_F(VulkanIntegrationTest, PreservesImageViewHandleAfterMoveConstruction) {
    const Vulkan::VulkanImage image{
        memoryAllocator(),
        makeImageCreateInfo(kColorFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT),
    };

    const VkImageViewCreateInfo create_info = makeImageViewCreateInfo(
        image.nativeHandle(), kColorFormat, VK_IMAGE_ASPECT_COLOR_BIT);

    Vulkan::VulkanImageView source{m_Device, create_info};

    const VkImageView original_handle = source.nativeHandle();

    ASSERT_NE(original_handle, VK_NULL_HANDLE);

    const Vulkan::VulkanImageView destination{std::move(source)};

    EXPECT_EQ(destination.nativeHandle(), original_handle);
}

TEST_F(VulkanIntegrationTest, TransfersImageViewOwnershipDuringMoveAssignment) {
    const Vulkan::VulkanImage image{
        memoryAllocator(),
        makeImageCreateInfo(kColorFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT),
    };

    const VkImageViewCreateInfo create_info = makeImageViewCreateInfo(
        image.nativeHandle(), kColorFormat, VK_IMAGE_ASPECT_COLOR_BIT);

    Vulkan::VulkanImageView source{m_Device, create_info};
    Vulkan::VulkanImageView destination{m_Device, create_info};

    const VkImageView source_handle = source.nativeHandle();
    const VkImageView destination_handle = destination.nativeHandle();

    ASSERT_NE(source_handle, VK_NULL_HANDLE);
    ASSERT_NE(destination_handle, VK_NULL_HANDLE);
    ASSERT_NE(source_handle, destination_handle);

    destination = std::move(source);

    EXPECT_EQ(destination.nativeHandle(), source_handle);
    EXPECT_NE(destination.nativeHandle(), destination_handle);
}

TEST(VulkanImageViewPreconditionTests, RejectsNullDevice) {
    const VkImageViewCreateInfo create_info{};

    EXPECT_DEATH(
        static_cast<void>(Vulkan::VulkanImageView(VK_NULL_HANDLE, create_info)),
        "VulkanImageView requires a valid Vulkan device");
}
