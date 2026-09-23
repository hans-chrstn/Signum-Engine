#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include <catch2/catch_test_macros.hpp>
#include <cstring>
#include <exception>

namespace Error = SNE::Engine::Core::Error;

TEST_CASE("EngineError preserves engine error code") {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::EngineError engine_error =
        Error::EngineError(code, "Failed to initialize GLFW!");

    REQUIRE(engine_error.getCode() == Error::Code::GlfwInitializationFailed);
}

TEST_CASE("EngineError preserves error message") {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::EngineError engine_error =
        Error::EngineError(code, "Failed to initialize GLFW!");

    REQUIRE(std::strcmp(engine_error.what(), "Failed to initialize GLFW!") ==
            0);
}

TEST_CASE("EngineError behaves as a standard exception") {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::EngineError engine_error =
        Error::EngineError(code, "Failed to initialize GLFW!");

    REQUIRE_THROWS_AS(throw engine_error, std::exception);
}

TEST_CASE("EngineError without native context has no NativeError") {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::EngineError engine_error =
        Error::EngineError(code, "Failed to initialize GLFW!");

    REQUIRE_FALSE(engine_error.getNativeError().has_value());
}

TEST_CASE("EngineError with native context has a NativeError") {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::NativeError native_error = Error::NativeError(123, "Test");

    Error::EngineError engine_error =
        Error::EngineError(code, "Failed to initialize GLFW!", native_error);

    REQUIRE(engine_error.getNativeError().has_value());
}

TEST_CASE("EngineError preserves contained native error code") {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::NativeError native_error = Error::NativeError(123, "Test");

    Error::EngineError engine_error =
        Error::EngineError(code, "Failed to initialize GLFW!", native_error);

    REQUIRE(engine_error.getNativeError().has_value());
    REQUIRE(engine_error.getNativeError()->getNativeCode() == 123);
}

TEST_CASE("EngineError preserves contained native error description") {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::NativeError native_error = Error::NativeError(123, "Test");

    Error::EngineError engine_error =
        Error::EngineError(code, "Failed to initialize GLFW!", native_error);

    REQUIRE(engine_error.getNativeError().has_value());
    REQUIRE(engine_error.getNativeError()->getNativeDescription() == "Test");
}

TEST_CASE("EngineError reports its subsystem") {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::EngineError engine_error =
        Error::EngineError(code, "Failed to initialize GLFW!");

    REQUIRE(engine_error.getSubsystem() == Error::Subsystem::Platform);
}

TEST_CASE("EngineError preserves operation context") {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::EngineError engine_error = Error::EngineError(
        code, "Failed to initialize GLFW!", "Test Operation");

    REQUIRE(engine_error.getOperation() == "Test Operation");
}

TEST_CASE("EngineError without operation context has an empty operation") {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::EngineError engine_error =
        Error::EngineError(code, "Failed to initialize GLFW!");

    REQUIRE(engine_error.getOperation().empty());
}

TEST_CASE("EngineError preserves native error and operation context") {
    Error::Code code = Error::Code::GlfwInitializationFailed;

    Error::NativeError native_error = Error::NativeError(123, "Test");

    Error::EngineError engine_error = Error::EngineError(
        code, "Failed to initialize GLFW!", native_error, "Test Operation");

    REQUIRE(engine_error.getNativeError().has_value());
    REQUIRE(engine_error.getNativeError()->getNativeDescription() == "Test");
    REQUIRE(engine_error.getOperation() == "Test Operation");
}
