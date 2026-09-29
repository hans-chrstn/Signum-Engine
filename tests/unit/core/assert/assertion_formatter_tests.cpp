#include "engine/core/assert/assertion_failure.hpp"
#include "engine/core/assert/assertion_formatter.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include <gtest/gtest.h>
#include <string>

namespace Assertion = SNE::Engine::Core::Assertion;
namespace Error = SNE::Engine::Core::Error;

TEST(AssertionFormatterTests, FormatsAssertionFailure) {
    const Assertion::AssertionType assertion_type =
        Assertion::AssertionType::Precondition;
    const Assertion::AssertionFailure assertion_failure =
        Assertion::AssertionFailure(assertion_type, Error::Subsystem::Core,
                                    "Test Assertion Failure");

    const std::string formatted =
        Assertion::formatDiagnostic(assertion_failure);

    EXPECT_NE(formatted.find("Assertion Failure"), std::string::npos);
}

TEST(AssertionFormatterTests, IncludesAssertionFields) {
    const Assertion::AssertionType assertion_type =
        Assertion::AssertionType::Precondition;
    const Assertion::AssertionFailure assertion_failure =
        Assertion::AssertionFailure(assertion_type, Error::Subsystem::Core,
                                    "Test Assertion Failure");

    const std::string formatted =
        Assertion::formatDiagnostic(assertion_failure);
    EXPECT_NE(formatted.find("  Type: Precondition"), std::string::npos);
    EXPECT_NE(formatted.find("  Subsystem: Core"), std::string::npos);
    EXPECT_NE(formatted.find("  Message: Test Assertion Failure"),
              std::string::npos);
    EXPECT_NE(formatted.find("  Location: "), std::string::npos);
    EXPECT_NE(formatted.find("  Function: "), std::string::npos);
}

TEST(AssertionFormatterTests, IncludesSourceLocation) {
    const Assertion::AssertionType assertion_type =
        Assertion::AssertionType::Precondition;
    const Assertion::AssertionFailure assertion_failure =
        Assertion::AssertionFailure(assertion_type, Error::Subsystem::Core,
                                    "Test Assertion Failure");

    const std::string formatted =
        Assertion::formatDiagnostic(assertion_failure);

    const auto location = assertion_failure.getSourceLocation();
    const std::string expected_location = std::string("Location: ") +
                                          location.file_name() + ":" +
                                          std::to_string(location.line());

    const std::string expected_function =
        std::string("Function: ") + location.function_name();

    EXPECT_NE(formatted.find(expected_location), std::string::npos);
    EXPECT_NE(formatted.find(expected_function), std::string::npos);
}
