#include "engine/core/assert/assertion_failure.hpp"
#include "engine/core/error/subsystem.hpp"
#include <gtest/gtest.h>
#include <source_location>
#include <string_view>

namespace Assertion = SNE::Engine::Core::Assertion;
namespace Error = SNE::Engine::Core::Error;

TEST(AssertionFailureTests, PreservesAssertionType) {
    const Assertion::AssertionType assertion_type =
        Assertion::AssertionType::Precondition;

    const Error::Subsystem subsystem = Error::Subsystem::Core;

    const Assertion::AssertionFailure assertion_failure =
        Assertion::AssertionFailure(assertion_type, subsystem,
                                    "Test Assertion Failure");
    EXPECT_EQ(assertion_failure.getType(), assertion_type);
}

TEST(AssertionFailureTests, PreservesSubsystem) {
    const Assertion::AssertionType assertion_type =
        Assertion::AssertionType::Precondition;

    const Error::Subsystem subsystem = Error::Subsystem::Core;

    const Assertion::AssertionFailure assertion_failure =
        Assertion::AssertionFailure(assertion_type, subsystem,
                                    "Test Assertion Failure");
    EXPECT_EQ(assertion_failure.getSubsystem(), subsystem);
}

TEST(AssertionFailureTests, PreservesMessage) {
    const Assertion::AssertionType assertion_type =
        Assertion::AssertionType::Precondition;

    const Error::Subsystem subsystem = Error::Subsystem::Core;

    const Assertion::AssertionFailure assertion_failure =
        Assertion::AssertionFailure(assertion_type, subsystem,
                                    "Test Assertion Failure");
    EXPECT_EQ(assertion_failure.getMessage(), "Test Assertion Failure");
}

TEST(AssertionFailureTests, PreservesSourceLocation) {
    const Assertion::AssertionType assertion_type =
        Assertion::AssertionType::Precondition;

    const Error::Subsystem subsystem = Error::Subsystem::Core;

    const Assertion::AssertionFailure assertion_failure =
        Assertion::AssertionFailure(assertion_type, subsystem,
                                    "Test Assertion Failure");

    const std::source_location source_location =
        assertion_failure.getSourceLocation();

    EXPECT_TRUE(std::string_view(source_location.file_name())
                    .ends_with("assertion_failure_tests.cpp"));

    EXPECT_GT(source_location.line(), 0U);

    EXPECT_FALSE(std::string_view(source_location.function_name()).empty());
}
