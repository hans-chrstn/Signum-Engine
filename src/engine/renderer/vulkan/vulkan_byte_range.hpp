#pragma once
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Describes a contiguous range of bytes.
     *
     * Represents a byte region using a starting offset and a size in bytes.
     *
     * This type describes a range only and does not own or reference the memory
     * represented by that range.
     */
    struct ByteRange {
        /** Starting byte offset of the range. */
        VkDeviceSize offset{};

        /** Number of bytes contained in the range. */
        VkDeviceSize size{};

        /**
         * @brief Determines whether this byte range fits within a total byte
         * size.
         *
         * Validates that this range lies entirely within a containing region of
         * the specified total size.
         *
         * Validation avoids forming the sum of the range offset and size so
         * that the check does not depend on potentially overflowing unsigned
         * arithmetic.
         *
         * A zero-sized range is valid when its offset does not exceed the total
         * size.
         *
         * @param total_size Total size in bytes of the containing region.
         *
         * @return true if this entire byte range lies within total_size;
         * otherwise false.
         */
        [[nodiscard]] auto fitsWithin(VkDeviceSize total_size) const noexcept
            -> bool;

        /**
         * @brief Determines whether this byte range overlaps another byte
         * range.
         *
         * Treats both ranges as half-open intervals. Two ranges that only touch
         * at their boundaries do not overlap. A zero-sized range does not
         * overlap any range.
         *
         * The overlap test avoids arithmetic that could overflow VkDeviceSize.
         *
         * @param other Byte range to compare against this range.
         *
         * @return true when the ranges share at least one byte; otherwise
         * false.
         */
        [[nodiscard]] auto overlaps(const ByteRange &other) const noexcept
            -> bool;
    };
} // namespace SNE::Engine::Renderer::Vulkan
