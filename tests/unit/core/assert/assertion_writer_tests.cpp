#include "engine/core/assert/assertion_failure.hpp"
#include "engine/core/assert/assertion_formatter.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/assert/assertion_writer.hpp"
#include "engine/core/error/subsystem.hpp"
#include <gtest/gtest.h>
#include <sstream>

namespace Assertion = SNE::Engine::Core::Assertion;
namespace Error = SNE::Engine::Core::Error;

TEST(AssertionWriterTests, WritesFormattedDiagnostic) {
    const Assertion::AssertionFailure assertion_failure =
        Assertion::AssertionFailure(Assertion::AssertionType::Precondition,
                                    Error::Subsystem::Core,
                                    "Test Assertion Failure");

    std::ostringstream output;

    Assertion::writeDiagnostic(output, assertion_failure);

    EXPECT_EQ(output.str(), Assertion::formatDiagnostic(assertion_failure));
}
