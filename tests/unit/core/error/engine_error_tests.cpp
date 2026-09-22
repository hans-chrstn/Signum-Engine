#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include <catch2/catch_test_macros.hpp>
#include <cstring>
#include <exception>

TEST_CASE("Successfully preserve EngineError error code") {
    SNE::Engine::Core::Error::Code code =
        SNE::Engine::Core::Error::Code::GlfwInitializationFailed;

    SNE::Engine::Core::Error::EngineError engine_error =
        SNE::Engine::Core::Error::EngineError(code,
                                              "Failed to initialize GLFW!");

    REQUIRE(engine_error.getCode() ==
            SNE::Engine::Core::Error::Code::GlfwInitializationFailed);
}

TEST_CASE("Successfully preserve EngineError message") {
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
