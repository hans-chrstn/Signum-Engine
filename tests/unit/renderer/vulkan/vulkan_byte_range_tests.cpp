#include "engine/renderer/vulkan/vulkan_byte_range.hpp"
#include <gtest/gtest.h>
#include <limits>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

TEST(ByteRangeTests, FitsWithinReturnsTrueForExactRange) {
    const VkDeviceSize offset = 5U;
    const VkDeviceSize size = 2U;
    const Vulkan::ByteRange byte_range{
        .offset = offset,
        .size = size,
    };

    EXPECT_TRUE(byte_range.fitsWithin(7U));
}

TEST(ByteRangeTests, FitsWithinReturnsTrueForRangeInsideBounds) {
    const VkDeviceSize offset = 5U;
    const VkDeviceSize size = 2U;
    const Vulkan::ByteRange byte_range{
        .offset = offset,
        .size = size,
    };

    EXPECT_TRUE(byte_range.fitsWithin(8U));
}

TEST(ByteRangeTests, FitsWithinReturnsTrueForZeroSizeAtEnd) {
    const VkDeviceSize offset = 5U;
    const VkDeviceSize size = 0U;
    const Vulkan::ByteRange byte_range{
        .offset = offset,
        .size = size,
    };

    EXPECT_TRUE(byte_range.fitsWithin(5U));
}

TEST(ByteRangeTests, FitsWithinReturnsFalseWhenOffsetExceedsTotalSize) {
    const VkDeviceSize offset = 6U;
    const VkDeviceSize size = 2U;
    const Vulkan::ByteRange byte_range{
        .offset = offset,
        .size = size,
    };

    EXPECT_FALSE(byte_range.fitsWithin(5U));
}

TEST(ByteRangeTests, FitsWithinReturnsFalseWhenRangeExceedsRemainingSize) {
    const VkDeviceSize offset = 5U;
    const VkDeviceSize size = 4U;
    const Vulkan::ByteRange byte_range{
        .offset = offset,
        .size = size,
    };

    EXPECT_FALSE(byte_range.fitsWithin(8U));
}

TEST(ByteRangeTests, FitsWithinHandlesLargeValuesWithoutOverflow) {
    const VkDeviceSize max = std::numeric_limits<VkDeviceSize>::max();
    const VkDeviceSize offset = max - 1;
    const VkDeviceSize size = 2U;
    const Vulkan::ByteRange byte_range{
        .offset = offset,
        .size = size,
    };

    EXPECT_FALSE(byte_range.fitsWithin(max));
}

TEST(ByteRangeTests, OverlapsReturnsTrueForPartialOverlap) {
    const Vulkan::ByteRange first{
        .offset = 0U,
        .size = 8U,
    };

    const Vulkan::ByteRange second{
        .offset = 4U,
        .size = 8U,
    };

    EXPECT_TRUE(first.overlaps(second));
}

TEST(ByteRangeTests, OverlapsReturnsTrueForIdenticalRanges) {
    const Vulkan::ByteRange first{
        .offset = 4U,
        .size = 8U,
    };

    const Vulkan::ByteRange second{
        .offset = 4U,
        .size = 8U,
    };

    EXPECT_TRUE(first.overlaps(second));
}

TEST(ByteRangeTests, OverlapsReturnsTrueWhenThisContainsOther) {
    const Vulkan::ByteRange first{
        .offset = 0U,
        .size = 16U,
    };

    const Vulkan::ByteRange second{
        .offset = 4U,
        .size = 4U,
    };

    EXPECT_TRUE(first.overlaps(second));
}

TEST(ByteRangeTests, OverlapsReturnsTrueWhenOtherContainsThis) {
    const Vulkan::ByteRange first{
        .offset = 4U,
        .size = 4U,
    };

    const Vulkan::ByteRange second{
        .offset = 0U,
        .size = 16U,
    };

    EXPECT_TRUE(first.overlaps(second));
}

TEST(ByteRangeTests, OverlapsReturnsFalseForTouchingRanges) {
    const Vulkan::ByteRange first{
        .offset = 0U,
        .size = 4U,
    };

    const Vulkan::ByteRange second{
        .offset = 4U,
        .size = 4U,
    };

    EXPECT_FALSE(first.overlaps(second));
}

TEST(ByteRangeTests, OverlapsReturnsFalseForSeparatedRanges) {
    const Vulkan::ByteRange first{
        .offset = 0U,
        .size = 4U,
    };

    const Vulkan::ByteRange second{
        .offset = 8U,
        .size = 4U,
    };

    EXPECT_FALSE(first.overlaps(second));
}

TEST(ByteRangeTests, OverlapsReturnsFalseWhenThisRangeIsEmpty) {
    const Vulkan::ByteRange first{
        .offset = 4U,
        .size = 0U,
    };

    const Vulkan::ByteRange second{
        .offset = 0U,
        .size = 8U,
    };

    EXPECT_FALSE(first.overlaps(second));
}

TEST(ByteRangeTests, OverlapsReturnsFalseWhenOtherRangeIsEmpty) {
    const Vulkan::ByteRange first{
        .offset = 0U,
        .size = 8U,
    };

    const Vulkan::ByteRange second{
        .offset = 4U,
        .size = 0U,
    };

    EXPECT_FALSE(first.overlaps(second));
}

TEST(ByteRangeTests, OverlapsHandlesLargeValuesWithoutOverflow) {
    constexpr VkDeviceSize max = std::numeric_limits<VkDeviceSize>::max();

    const Vulkan::ByteRange first{
        .offset = max - 3U,
        .size = 4U,
    };

    const Vulkan::ByteRange second{
        .offset = max - 1U,
        .size = 2U,
    };

    EXPECT_TRUE(first.overlaps(second));
}
