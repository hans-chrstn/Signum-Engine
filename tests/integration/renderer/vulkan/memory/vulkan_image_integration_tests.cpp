#include "../vulkan_integration_test_fixture.hpp"
#include "engine/renderer/vulkan/memory/vulkan_image.hpp"
#include "engine/renderer/vulkan/memory/vulkan_image_view.hpp"
#include "engine/renderer/vulkan/memory/vulkan_memory_allocator.hpp"
#include <cstdint>
#include <gtest/gtest.h>
#include <utility>
#include <vulkan/vulkan.h>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    constexpr std::uint32_t kTestImageExtent = 8U;
    constexpr std::uint32_t kTestImageDepth = 1U;
    constexpr std::uint32_t kCubeFaceCount = 6U;

    [[nodiscard]] auto makeTestImageCreateInfo()
        -> Vulkan::VulkanImageCreateInfo {
        return Vulkan::VulkanImageCreateInfo{
            .extent =
                {
                    .width = kTestImageExtent,
                    .height = kTestImageExtent,
                    .depth = kTestImageDepth,
                },
            .image_type = VK_IMAGE_TYPE_2D,
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .usage = VK_IMAGE_USAGE_SAMPLED_BIT,
        };
    }
} // namespace

TEST_F(VulkanIntegrationTest, CreatesAndDestroysVmaBackedImage) {
    const std::uint32_t width = 64U;
    const std::uint32_t height = 64U;
    const std::uint32_t depth = 1U;

    const Vulkan::VulkanImageCreateInfo image_create_info{
        .extent =
            {
                .width = width,
                .height = height,
                .depth = depth,
            },
        .image_type = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    };

    Vulkan::VulkanImage image =
        Vulkan::VulkanImage(memoryAllocator(), image_create_info);
}

TEST_F(VulkanIntegrationTest, MoveConstructsVmaBackedImage) {
    const Vulkan::VulkanImageCreateInfo image_create_info{
        .extent =
            {
                .width = 64U,
                .height = 64U,
                .depth = 1U,
            },
        .image_type = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    };

    const Vulkan::VulkanMemoryAllocationStatistics statistics_before =
        memoryAllocator().queryMemoryAllocationStatistics();

    {
        Vulkan::VulkanImage source{memoryAllocator(), image_create_info};

        const Vulkan::VulkanMemoryAllocationStatistics statistics_before_move =
            memoryAllocator().queryMemoryAllocationStatistics();

        EXPECT_EQ(statistics_before_move.allocation_count,
                  statistics_before.allocation_count + 1U);

        Vulkan::VulkanImage destination{std::move(source)};

        const Vulkan::VulkanMemoryAllocationStatistics statistics_after_move =
            memoryAllocator().queryMemoryAllocationStatistics();

        EXPECT_EQ(statistics_after_move.allocation_count,
                  statistics_before.allocation_count + 1U);
    }

    const Vulkan::VulkanMemoryAllocationStatistics
        statistics_after_destruction =
            memoryAllocator().queryMemoryAllocationStatistics();

    EXPECT_EQ(statistics_after_destruction.allocation_count,
              statistics_before.allocation_count);
}

TEST_F(VulkanIntegrationTest, MoveAssignsVmaBackedImage) {
    const Vulkan::VulkanImageCreateInfo source_create_info{
        .extent =
            {
                .width = 64U,
                .height = 64U,
                .depth = 1U,
            },
        .image_type = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    };

    const Vulkan::VulkanImageCreateInfo destination_create_info{
        .extent =
            {
                .width = 32U,
                .height = 32U,
                .depth = 1U,
            },
        .image_type = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
    };

    const Vulkan::VulkanMemoryAllocationStatistics statistics_before =
        memoryAllocator().queryMemoryAllocationStatistics();

    {
        Vulkan::VulkanImage source{memoryAllocator(), source_create_info};

        Vulkan::VulkanImage destination{memoryAllocator(),
                                        destination_create_info};

        const Vulkan::VulkanMemoryAllocationStatistics statistics_before_move =
            memoryAllocator().queryMemoryAllocationStatistics();

        EXPECT_EQ(statistics_before_move.allocation_count,
                  statistics_before.allocation_count + 2U);

        destination = std::move(source);

        const Vulkan::VulkanMemoryAllocationStatistics statistics_after_move =
            memoryAllocator().queryMemoryAllocationStatistics();

        EXPECT_EQ(statistics_after_move.allocation_count,
                  statistics_before.allocation_count + 1U);
    }

    const Vulkan::VulkanMemoryAllocationStatistics
        statistics_after_destruction =
            memoryAllocator().queryMemoryAllocationStatistics();

    EXPECT_EQ(statistics_after_destruction.allocation_count,
              statistics_before.allocation_count);
}

TEST_F(VulkanIntegrationTest, NativeHandleReturnsCreatedImage) {
    const Vulkan::VulkanImageCreateInfo create_info{
        .extent =
            {
                .width = 64U,
                .height = 64U,
                .depth = 1U,
            },
        .image_type = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    };

    const Vulkan::VulkanImage image{memoryAllocator(), create_info};

    EXPECT_NE(image.nativeHandle(), VK_NULL_HANDLE);
}

TEST_F(VulkanIntegrationTest, NativeHandlePreservedAfterMoveConstruction) {
    const Vulkan::VulkanImageCreateInfo create_info{
        .extent =
            {
                .width = 64U,
                .height = 64U,
                .depth = 1U,
            },
        .image_type = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    };

    Vulkan::VulkanImage source{memoryAllocator(), create_info};

    const VkImage original_handle = source.nativeHandle();
    ASSERT_NE(original_handle, VK_NULL_HANDLE);

    const Vulkan::VulkanImage destination{std::move(source)};

    EXPECT_EQ(destination.nativeHandle(), original_handle);
}

TEST_F(VulkanIntegrationTest, NativeHandleUpdatedAfterMoveAssignment) {
    const Vulkan::VulkanImageCreateInfo create_info{
        .extent =
            {
                .width = 64U,
                .height = 64U,
                .depth = 1U,
            },
        .image_type = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    };

    Vulkan::VulkanImage source{memoryAllocator(), create_info};
    Vulkan::VulkanImage destination{memoryAllocator(), create_info};

    const VkImage source_handle = source.nativeHandle();
    const VkImage destination_handle = destination.nativeHandle();

    ASSERT_NE(source_handle, VK_NULL_HANDLE);
    ASSERT_NE(destination_handle, VK_NULL_HANDLE);
    ASSERT_NE(source_handle, destination_handle);

    destination = std::move(source);

    EXPECT_EQ(destination.nativeHandle(), source_handle);
    EXPECT_NE(destination.nativeHandle(), destination_handle);
}

TEST_F(VulkanIntegrationTest, CreatesImageWithMultipleMipLevels) {
    constexpr std::uint32_t kImageWidth = 8U;
    constexpr std::uint32_t kImageHeight = 8U;
    constexpr std::uint32_t kImageDepth = 1U;
    constexpr std::uint32_t kMipLevels = 4U;
    constexpr VkFormat kImageFormat = VK_FORMAT_R8G8B8A8_UNORM;

    const Vulkan::VulkanImageCreateInfo create_info{
        .extent =
            {
                .width = kImageWidth,
                .height = kImageHeight,
                .depth = kImageDepth,
            },
        .image_type = VK_IMAGE_TYPE_2D,
        .format = kImageFormat,
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT,
        .mip_levels = kMipLevels,
    };

    const Vulkan::VulkanMemoryAllocationStatistics statistics_before =
        memoryAllocator().queryMemoryAllocationStatistics();

    {
        const Vulkan::VulkanImage image{memoryAllocator(), create_info};

        ASSERT_NE(image.nativeHandle(), VK_NULL_HANDLE);

        VkImageViewCreateInfo view_create_info{};
        view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_create_info.image = image.nativeHandle();
        view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_create_info.format = kImageFormat;
        view_create_info.subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = kMipLevels - 1U,
            .levelCount = 1U,
            .baseArrayLayer = 0U,
            .layerCount = 1U,
        };

        const Vulkan::VulkanImageView image_view{m_Device, view_create_info};

        EXPECT_NE(image_view.nativeHandle(), VK_NULL_HANDLE);
    }

    const Vulkan::VulkanMemoryAllocationStatistics statistics_after =
        memoryAllocator().queryMemoryAllocationStatistics();

    EXPECT_EQ(statistics_after.allocation_count,
              statistics_before.allocation_count);
}

TEST_F(VulkanIntegrationTest, RejectsZeroMipLevelCount) {
    constexpr std::uint32_t kImageExtent = 8U;
    constexpr std::uint32_t kImageDepth = 1U;
    constexpr std::uint32_t kInvalidMipLevels = 0U;

    const Vulkan::VulkanImageCreateInfo create_info{
        .extent =
            {
                .width = kImageExtent,
                .height = kImageExtent,
                .depth = kImageDepth,
            },
        .image_type = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT,
        .mip_levels = kInvalidMipLevels,
    };

    ASSERT_DEATH(
        static_cast<void>(Vulkan::VulkanImage{memoryAllocator(), create_info}),
        "VulkanImage requires at least one mip level");
}

TEST_F(VulkanIntegrationTest, RejectsMipLevelsExceedingImageDimensions) {
    constexpr std::uint32_t kImageExtent = 8U;
    constexpr std::uint32_t kImageDepth = 1U;
    constexpr std::uint32_t kMaximumMipLevels = 4U;
    constexpr std::uint32_t kInvalidMipLevels = kMaximumMipLevels + 1U;

    const Vulkan::VulkanImageCreateInfo create_info{
        .extent =
            {
                .width = kImageExtent,
                .height = kImageExtent,
                .depth = kImageDepth,
            },
        .image_type = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT,
        .mip_levels = kInvalidMipLevels,
    };

    ASSERT_DEATH(
        static_cast<void>(Vulkan::VulkanImage{memoryAllocator(), create_info}),
        "VulkanImage mip level count exceeds the image dimensions");
}

TEST_F(VulkanIntegrationTest, CreatesImageWithMultipleArrayLayers) {
    constexpr std::uint32_t kArrayLayers = 4U;

    Vulkan::VulkanImageCreateInfo create_info = makeTestImageCreateInfo();
    create_info.array_layers = kArrayLayers;

    const Vulkan::VulkanImage image{memoryAllocator(), create_info};
    ASSERT_NE(image.nativeHandle(), VK_NULL_HANDLE);

    VkImageViewCreateInfo view_create_info{};
    view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_create_info.image = image.nativeHandle();
    view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    view_create_info.format = create_info.format;
    view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_create_info.subresourceRange.baseMipLevel = 0U;
    view_create_info.subresourceRange.levelCount = 1U;
    view_create_info.subresourceRange.baseArrayLayer = kArrayLayers - 1U;
    view_create_info.subresourceRange.layerCount = 1U;

    const Vulkan::VulkanImageView image_view{m_Device, view_create_info};
    EXPECT_NE(image_view.nativeHandle(), VK_NULL_HANDLE);
}

TEST_F(VulkanIntegrationTest, CreatesCubeCompatibleImage) {
    Vulkan::VulkanImageCreateInfo create_info = makeTestImageCreateInfo();
    create_info.array_layers = kCubeFaceCount;
    create_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    const Vulkan::VulkanImage image{memoryAllocator(), create_info};
    ASSERT_NE(image.nativeHandle(), VK_NULL_HANDLE);

    VkImageViewCreateInfo view_create_info{};
    view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_create_info.image = image.nativeHandle();
    view_create_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    view_create_info.format = create_info.format;
    view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_create_info.subresourceRange.baseMipLevel = 0U;
    view_create_info.subresourceRange.levelCount = 1U;
    view_create_info.subresourceRange.baseArrayLayer = 0U;
    view_create_info.subresourceRange.layerCount = kCubeFaceCount;

    const Vulkan::VulkanImageView image_view{m_Device, view_create_info};
    EXPECT_NE(image_view.nativeHandle(), VK_NULL_HANDLE);
}

TEST_F(VulkanIntegrationTest, CreatesMultisampledImageWhenSupported) {
    constexpr VkSampleCountFlagBits kSampleCount = VK_SAMPLE_COUNT_4_BIT;

    VkImageFormatProperties properties{};

    const VkResult query_result = vkGetPhysicalDeviceImageFormatProperties(
        m_PhysicalDevice, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_2D,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0U,
        &properties);

    if (query_result == VK_ERROR_FORMAT_NOT_SUPPORTED) {
        GTEST_SKIP() << "Requested image configuration is unsupported";
    }

    ASSERT_EQ(query_result, VK_SUCCESS);

    if ((properties.sampleCounts &
         static_cast<VkSampleCountFlags>(kSampleCount)) == 0U) {
        GTEST_SKIP() << "Four-sample images are unsupported";
    }

    Vulkan::VulkanImageCreateInfo create_info = makeTestImageCreateInfo();
    create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    create_info.samples = kSampleCount;

    const Vulkan::VulkanImage image{memoryAllocator(), create_info};

    EXPECT_NE(image.nativeHandle(), VK_NULL_HANDLE);
}

TEST_F(VulkanIntegrationTest, RejectsInvalidArrayLayerCounts) {
    constexpr std::uint32_t kInvalid3DArrayLayers = 2U;

    Vulkan::VulkanImageCreateInfo create_info = makeTestImageCreateInfo();

    create_info.array_layers = 0U;

    ASSERT_DEATH(
        static_cast<void>(Vulkan::VulkanImage{memoryAllocator(), create_info}),
        "VulkanImage requires at least one array layer");

    create_info.image_type = VK_IMAGE_TYPE_3D;
    create_info.array_layers = kInvalid3DArrayLayers;

    ASSERT_DEATH(
        static_cast<void>(Vulkan::VulkanImage{memoryAllocator(), create_info}),
        "VulkanImage requires exactly one array layer for 3D images");
}

TEST_F(VulkanIntegrationTest, RejectsInvalidCubeCompatibleImages) {
    Vulkan::VulkanImageCreateInfo create_info = makeTestImageCreateInfo();
    create_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    create_info.array_layers = kCubeFaceCount;

    create_info.image_type = VK_IMAGE_TYPE_1D;
    create_info.extent.height = 1U;

    ASSERT_DEATH(
        static_cast<void>(Vulkan::VulkanImage{memoryAllocator(), create_info}),
        "VulkanImage cube-compatible images must be 2D");

    create_info.image_type = VK_IMAGE_TYPE_2D;
    create_info.extent.height = kTestImageExtent / 2U;

    ASSERT_DEATH(
        static_cast<void>(Vulkan::VulkanImage{memoryAllocator(), create_info}),
        "VulkanImage cube-compatible images require equal width and height");

    create_info.extent.height = kTestImageExtent;
    create_info.array_layers = kCubeFaceCount - 1U;

    ASSERT_DEATH(
        static_cast<void>(Vulkan::VulkanImage{memoryAllocator(), create_info}),
        "VulkanImage cube-compatible images require at least six array layers");
}

TEST_F(VulkanIntegrationTest, RejectsInvalidMultisampledImages) {
    constexpr VkSampleCountFlagBits kSampleCount = VK_SAMPLE_COUNT_4_BIT;
    constexpr std::uint32_t kInvalidMipLevels = 2U;

    Vulkan::VulkanImageCreateInfo create_info = makeTestImageCreateInfo();
    create_info.samples = kSampleCount;

    create_info.image_type = VK_IMAGE_TYPE_1D;
    create_info.extent.height = 1U;

    ASSERT_DEATH(
        static_cast<void>(Vulkan::VulkanImage{memoryAllocator(), create_info}),
        "VulkanImage multisampled images must be 2D");

    create_info.image_type = VK_IMAGE_TYPE_2D;
    create_info.extent.height = kTestImageExtent;
    create_info.mip_levels = kInvalidMipLevels;

    ASSERT_DEATH(
        static_cast<void>(Vulkan::VulkanImage{memoryAllocator(), create_info}),
        "VulkanImage multisampled images require exactly one mip level");

    create_info.mip_levels = 1U;
    create_info.array_layers = kCubeFaceCount;
    create_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    ASSERT_DEATH(
        static_cast<void>(Vulkan::VulkanImage{memoryAllocator(), create_info}),
        "VulkanImage multisampled images cannot be cube-compatible");
}
