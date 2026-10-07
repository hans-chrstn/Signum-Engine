#include "engine/core/numeric/checked_conversion.hpp"
#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>
#include <limits>
#include <optional>

namespace Numeric = SNE::Engine::Core::Numeric;

TEST(CheckedConversionTests, TryConvertToUint32ReturnsZero) {
    EXPECT_EQ(Numeric::tryConvertToUint32(std::size_t{0}), std::uint32_t{0});
}

TEST(CheckedConversionTests, TryConvertToUint32ReturnsNormalValue) {
    EXPECT_EQ(Numeric::tryConvertToUint32(std::size_t{5}), std::uint32_t{5});
}

TEST(CheckedConversionTests, TryConvertToUint32ReturnsUint32Maximum) {
    constexpr auto max = std::numeric_limits<std::uint32_t>::max();
    EXPECT_EQ(Numeric::tryConvertToUint32(std::size_t{max}), max);
}

TEST(CheckedConversionTests,
     TryConvertToUint32ReturnsNulloptWhenValueExceedsUint32) {
    constexpr auto max = std::numeric_limits<std::uint32_t>::max();
    if constexpr (std::numeric_limits<std::size_t>::max() > max) {
        const auto oversize_val = std::size_t{max} + 1U;
        EXPECT_EQ(Numeric::tryConvertToUint32(oversize_val), std::nullopt);
    } else {
        GTEST_SKIP() << "std::size_t cannot represent values greater than "
                        "std::uint32_t on this platform";
    }
}
