#include "engine/core/error/diagnostic_formatter.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_boundary.hpp"
#include <cstdlib>
#include <exception>
#include <gtest/gtest.h>
#include <sstream>
#include <stdexcept>

namespace Error = SNE::Engine::Core::Error;

TEST(ErrorBoundaryTests, ReturnsSuccessWhenOperationCompletes) {
    std::ostringstream diagnostics;
    int invocation_count{};

    const int result = Error::runWithErrorBoundary(
        [&] -> void { ++invocation_count; }, diagnostics);

    EXPECT_EQ(result, EXIT_SUCCESS);
    EXPECT_EQ(invocation_count, 1);
    EXPECT_TRUE(diagnostics.str().empty());
}

TEST(ErrorBoundaryTests, ReportsEngineErrorOnce) {
    const Error::EngineError error{
        Error::Code::GlfwInitializationFailed,
        "Failed to initialize GLFW",
        "Initialize GLFW",
    };

    const std::exception_ptr exception = std::make_exception_ptr(error);

    std::ostringstream diagnostics;

    const int result = Error::runWithErrorBoundary(
        [&] -> void { std::rethrow_exception(exception); }, diagnostics);

    EXPECT_EQ(result, EXIT_FAILURE);
    EXPECT_EQ(diagnostics.str(), Error::formatDiagnostic(error));
}

TEST(ErrorBoundaryTests, ReportsUnexpectedStandardException) {
    std::ostringstream diagnostics;

    const int result = Error::runWithErrorBoundary(
        [] -> void { throw std::runtime_error{"Unexpected failure"}; },
        diagnostics);

    EXPECT_EQ(result, EXIT_FAILURE);
    EXPECT_EQ(diagnostics.str(),
              "Unexpected standard exception: Unexpected failure\n");
}

TEST(ErrorBoundaryTests, ReportsUnknownException) {
    struct UnknownFailure {};

    const std::exception_ptr exception =
        std::make_exception_ptr(UnknownFailure{});

    std::ostringstream diagnostics;

    const int result = Error::runWithErrorBoundary(
        [&] -> void { std::rethrow_exception(exception); }, diagnostics);

    EXPECT_EQ(result, EXIT_FAILURE);
    EXPECT_EQ(diagnostics.str(),
              "Unknown exception reached application boundary\n");
}
