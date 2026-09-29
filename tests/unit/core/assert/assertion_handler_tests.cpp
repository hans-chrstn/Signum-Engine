#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include <gtest/gtest.h>

namespace Assertion = SNE::Engine::Core::Assertion;
namespace Error = SNE::Engine::Core::Error;

TEST(AssertionHandlerTests, FailsFastWithDiagnostic) {
    ASSERT_DEATH(Assertion::failAssertion(
                     Assertion::AssertionType::Precondition,
                     Error::Subsystem::Core, "Test Assertion Failure"),
                 "Test Assertion Failure");
}

TEST(AssertionHandlerTests, ReportsCallerSourceLocation) {
    ASSERT_DEATH(Assertion::failAssertion(
                     Assertion::AssertionType::Precondition,
                     Error::Subsystem::Core, "Test Assertion Failure"),
                 "assertion_handler_tests\\.cpp");
}
