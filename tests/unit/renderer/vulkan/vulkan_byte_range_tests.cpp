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
