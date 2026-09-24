#include "engine/core/error/diagnostic_formatter.hpp"
#include "engine/core/error/diagnostic_writer.hpp"
#include "engine/core/error/engine_error.hpp"
#include <gtest/gtest.h>
#include <sstream>

namespace Error = SNE::Engine::Core::Error;

TEST(DiagnosticWriterTests, WritesFormattedDiagnostic) {
    Error::EngineError engine_error =
        Error::EngineError(Error::Code::GlfwInitializationFailed,
                           "Failed to Initialize GLFW", "Initialize GLFW");

    std::ostringstream output;

    Error::writeDiagnostic(output, engine_error);

    EXPECT_EQ(output.str(), Error::formatDiagnostic(engine_error));
}
