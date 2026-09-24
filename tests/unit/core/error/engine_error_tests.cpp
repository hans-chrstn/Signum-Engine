#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include <exception>
#include <gtest/gtest.h>
#include <string_view>

namespace Error = SNE::Engine::Core::Error;
constexpr int kTestNativeCode = 123;

TEST(EngineErrorTests, PreservesEngineErrorCode) {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::EngineError engine_error =
        Error::EngineError(code, "Failed to initialize GLFW!");

    EXPECT_EQ(engine_error.getCode(), code);
}

TEST(EngineErrorTests, PreservesEngineErrorMessage) {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::EngineError engine_error =
        Error::EngineError(code, "Failed to initialize GLFW!");

    EXPECT_STREQ(engine_error.what(), "Failed to initialize GLFW!");
}

TEST(EngineErrorTests, BehavesAsStandardException) {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::EngineError engine_error =
        Error::EngineError(code, "Failed to initialize GLFW!");

    EXPECT_THROW(throw engine_error, std::exception);
}

TEST(EngineErrorTests, WithoutNativeContextHasNoNativeError) {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::EngineError engine_error =
        Error::EngineError(code, "Failed to initialize GLFW!");

    EXPECT_FALSE(engine_error.getNativeError().has_value());
}

TEST(EngineErrorTests, WithNativeContextHasNativeError) {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::NativeError native_error =
        Error::NativeError(kTestNativeCode, "Test");

    Error::EngineError engine_error =
        Error::EngineError(code, "Failed to initialize GLFW!", native_error);

    EXPECT_TRUE(engine_error.getNativeError().has_value());
}

TEST(EngineErrorTests, PreservesContainedNativeErrorCode) {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::NativeError native_error =
        Error::NativeError(kTestNativeCode, "Test");

    Error::EngineError engine_error =
        Error::EngineError(code, "Failed to initialize GLFW!", native_error);

    const auto &native = engine_error.getNativeError();

    ASSERT_TRUE(native.has_value());

    if (native.has_value()) {
        EXPECT_EQ(native->getNativeCode(), kTestNativeCode);
    }
}

TEST(EngineErrorTests, PreservesContainedNativeErrorDescription) {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::NativeError native_error =
        Error::NativeError(kTestNativeCode, "Test");

    Error::EngineError engine_error =
        Error::EngineError(code, "Failed to initialize GLFW!", native_error);

    const auto &native = engine_error.getNativeError();

    ASSERT_TRUE(native.has_value());

    if (native.has_value()) {
        EXPECT_EQ(native->getNativeCode(), kTestNativeCode);
    }
}

TEST(EngineErrorTests, ReportsItsSubsystem) {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::EngineError engine_error =
        Error::EngineError(code, "Failed to initialize GLFW!");

    EXPECT_EQ(engine_error.getSubsystem(), Error::Subsystem::Platform);
}

TEST(EngineErrorTests, PreservesOperationContext) {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::EngineError engine_error = Error::EngineError(
        code, "Failed to initialize GLFW!", "Test Operation");

    EXPECT_EQ(engine_error.getOperation(), "Test Operation");
}

TEST(EngineErrorTests, WithoutOperationContextHasEmptyOperation) {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::EngineError engine_error =
        Error::EngineError(code, "Failed to initialize GLFW!");

    EXPECT_TRUE(engine_error.getOperation().empty());
}

TEST(EngineErrorTests, PreservesNativeErrorAndOperationContext) {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::NativeError native_error =
        Error::NativeError(kTestNativeCode, "Test");

    Error::EngineError engine_error = Error::EngineError(
        code, "Failed to initialize GLFW!", native_error, "Test Operation");

    const auto &native = engine_error.getNativeError();

    ASSERT_TRUE(native.has_value());

    if (native.has_value()) {
        EXPECT_EQ(native->getNativeCode(), kTestNativeCode);
    }

    EXPECT_EQ(engine_error.getOperation(), "Test Operation");
}

TEST(EngineErrorTests, PreservesSourceLocation) {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::EngineError engine_error = Error::EngineError(
        code, "Failed to initialize GLFW!", "Test Operation");

    EXPECT_TRUE(std::string_view(engine_error.getSourceLocation().file_name())
                    .ends_with("engine_error_tests.cpp"));

    EXPECT_GT(engine_error.getSourceLocation().line(), 0U);

    EXPECT_FALSE(
        std::string_view(engine_error.getSourceLocation().function_name())
            .empty());
}
