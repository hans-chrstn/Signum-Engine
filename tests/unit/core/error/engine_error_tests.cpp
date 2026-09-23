#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include <catch2/catch_test_macros.hpp>
#include <cstring>
#include <exception>

TEST_CASE("EngineError preserves engine error code") {
    SNE::Engine::Core::Error::Code code =
        SNE::Engine::Core::Error::Code::GlfwInitializationFailed;

    SNE::Engine::Core::Error::EngineError engine_error =
        SNE::Engine::Core::Error::EngineError(code,
                                              "Failed to initialize GLFW!");

    REQUIRE(engine_error.getCode() ==
            SNE::Engine::Core::Error::Code::GlfwInitializationFailed);
}

TEST_CASE("EngineError preserves error message") {
    SNE::Engine::Core::Error::Code code =
        SNE::Engine::Core::Error::Code::GlfwInitializationFailed;

    SNE::Engine::Core::Error::EngineError engine_error =
        SNE::Engine::Core::Error::EngineError(code,
                                              "Failed to initialize GLFW!");

    REQUIRE(std::strcmp(engine_error.what(), "Failed to initialize GLFW!") ==
            0);
}

TEST_CASE("EngineError behaves as a standard exception") {
    SNE::Engine::Core::Error::Code code =
        SNE::Engine::Core::Error::Code::GlfwInitializationFailed;

    SNE::Engine::Core::Error::EngineError engine_error =
        SNE::Engine::Core::Error::EngineError(code,
                                              "Failed to initialize GLFW!");

    REQUIRE_THROWS_AS(throw engine_error, std::exception);
}

TEST_CASE("EngineError without native context has no NativeError") {
    SNE::Engine::Core::Error::Code code =
        SNE::Engine::Core::Error::Code::GlfwInitializationFailed;

    SNE::Engine::Core::Error::EngineError engine_error =
        SNE::Engine::Core::Error::EngineError(code,
                                              "Failed to initialize GLFW!");

    REQUIRE_FALSE(engine_error.getNativeError().has_value());
}

TEST_CASE("EngineError with native context has a NativeError") {
    SNE::Engine::Core::Error::Code code =
        SNE::Engine::Core::Error::Code::GlfwInitializationFailed;

    SNE::Engine::Core::Error::NativeError native_error =
        SNE::Engine::Core::Error::NativeError(123, "Test");

    SNE::Engine::Core::Error::EngineError engine_error =
        SNE::Engine::Core::Error::EngineError(
            code, "Failed to initialize GLFW!", native_error);

    REQUIRE(engine_error.getNativeError().has_value());
}

TEST_CASE("EngineError preserves contained native error code") {
    SNE::Engine::Core::Error::Code code =
        SNE::Engine::Core::Error::Code::GlfwInitializationFailed;

    SNE::Engine::Core::Error::NativeError native_error =
        SNE::Engine::Core::Error::NativeError(123, "Test");

    SNE::Engine::Core::Error::EngineError engine_error =
        SNE::Engine::Core::Error::EngineError(
            code, "Failed to initialize GLFW!", native_error);

    REQUIRE(engine_error.getNativeError()->getNativeCode() == 123);
}

TEST_CASE("EngineError preserves contained native error description") {
    SNE::Engine::Core::Error::Code code =
        SNE::Engine::Core::Error::Code::GlfwInitializationFailed;

    SNE::Engine::Core::Error::NativeError native_error =
        SNE::Engine::Core::Error::NativeError(123, "Test");

    SNE::Engine::Core::Error::EngineError engine_error =
        SNE::Engine::Core::Error::EngineError(
            code, "Failed to initialize GLFW!", native_error);

    REQUIRE(engine_error.getNativeError()->getNativeDescription() == "Test");
}
