#include "engine/core/assert/assertion_type.hpp"
#include <cstdint>
#include <gtest/gtest.h>

namespace Assertion = SNE::Engine::Core::Assertion;
constexpr std::uint8_t kUnknownAssertionTypeValue = 255;

TEST(AssertionTypeTests, PreconditionAssertionTypeHasReadableName) {
    EXPECT_EQ(Assertion::toString(Assertion::AssertionType::Precondition),
              "Precondition");
}

TEST(AssertionTypeTests, PostconditionAssertionTypeHasReadableName) {
    EXPECT_EQ(Assertion::toString(Assertion::AssertionType::Postcondition),
              "Postcondition");
}

TEST(AssertionTypeTests, InvariantAssertionTypeHasReadableName) {
    EXPECT_EQ(Assertion::toString(Assertion::AssertionType::Invariant),
              "Invariant");
}

TEST(AssertionTypeTests, UnknownAssertionTypeHasReadableName) {
    const auto invalid_assertion_type =
        // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
        static_cast<Assertion::AssertionType>(kUnknownAssertionTypeValue);
    EXPECT_EQ(Assertion::toString(invalid_assertion_type), "Unknown");
}
