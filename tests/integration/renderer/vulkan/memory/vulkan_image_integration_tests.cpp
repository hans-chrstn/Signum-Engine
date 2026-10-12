#include "../vulkan_integration_test_fixture.hpp"
#include "engine/renderer/vulkan/memory/vulkan_image.hpp"
#include "engine/renderer/vulkan/memory/vulkan_memory_allocator.hpp"
#include <cstdint>
#include <gtest/gtest.h>
#include <utility>
#include <vulkan/vulkan.h>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

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
