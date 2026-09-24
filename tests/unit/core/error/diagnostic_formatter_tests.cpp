#include "engine/core/error/diagnostic_formatter.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include <gtest/gtest.h>
#include <string>

namespace Error = SNE::Engine::Core::Error;
constexpr int kTestDiagnosticFormatterCode = 123;

TEST(DiagnosticFormatterTests, FormatsEngineErrorWithoutNativeContext) {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::EngineError engine_error = Error::EngineError(
        code, "Failed to initialize GLFW", "Initialize GLFW");

    const std::string formatted = Error::formatDiagnostic(engine_error);

    EXPECT_EQ(formatted.find("Native Code:"), std::string::npos);
    EXPECT_EQ(formatted.find("Native Info:"), std::string::npos);
}

TEST(DiagnosticFormatterTests, FormatsEngineErrorWithNativeContext) {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::NativeError native_error =
        Error::NativeError(kTestDiagnosticFormatterCode, "Test Description");

    Error::EngineError engine_error = Error::EngineError(
        code, "Failed to initialize GLFW", native_error, "Initialize GLFW");

    const std::string formatted = Error::formatDiagnostic(engine_error);

    EXPECT_NE(formatted.find("Native Code:"), std::string::npos);
    EXPECT_NE(formatted.find("Native Info:"), std::string::npos);
}

TEST(DiagnosticFormatterTests, IncludesNativeErrorValues) {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::NativeError native_error =
        Error::NativeError(kTestDiagnosticFormatterCode, "Test Description");

    Error::EngineError engine_error = Error::EngineError(
        code, "Failed to initialize GLFW", native_error, "Initialize GLFW");

    const std::string formatted = Error::formatDiagnostic(engine_error);

    EXPECT_NE(formatted.find("Native Code: 123"), std::string::npos);
    EXPECT_NE(formatted.find("Native Info: Test Description"),
              std::string::npos);
}

TEST(DiagnosticFormatterTests, IncludesCoreEngineErrorFields) {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::EngineError engine_error = Error::EngineError(
        code, "Failed to initialize GLFW", "Initialize GLFW");

    const std::string formatted = Error::formatDiagnostic(engine_error);

    EXPECT_NE(formatted.find("Engine Error"), std::string::npos);
    EXPECT_NE(formatted.find("Subsystem: Platform"), std::string::npos);
    EXPECT_NE(formatted.find("Operation: Initialize GLFW"), std::string::npos);
    EXPECT_NE(formatted.find("Code: GlfwInitializationFailed"),
              std::string::npos);
    EXPECT_NE(formatted.find("Message: Failed to initialize GLFW"),
              std::string::npos);
}

TEST(DiagnosticFormatterTests, IncludesSourceLocation) {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::EngineError engine_error = Error::EngineError(
        code, "Failed to initialize GLFW", "Initialize GLFW");

    const std::string formatted = Error::formatDiagnostic(engine_error);

    const auto location = engine_error.getSourceLocation();
    const std::string expected_location = std::string("Location: ") +
                                          location.file_name() + ":" +
                                          std::to_string(location.line());

    const std::string expected_function =
        std::string("Function: ") + location.function_name();

    EXPECT_NE(formatted.find(expected_location), std::string::npos);
    EXPECT_NE(formatted.find(expected_function), std::string::npos);
}
