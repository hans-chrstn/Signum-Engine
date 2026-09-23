#include "engine/core/error/error_code.hpp"
#include <catch2/catch_test_macros.hpp>

namespace Error = SNE::Engine::Core::Error;

TEST_CASE("Error code names are readable") {
    Error::Code glfw_error_code = Error::Code::GlfwInitializationFailed;
    REQUIRE(Error::toString(glfw_error_code) == "GlfwInitializationFailed");

    Error::Code window_error_code = Error::Code::WindowCreationFailed;
    REQUIRE(Error::toString(window_error_code) == "WindowCreationFailed");
}

TEST_CASE("Unknown error codes have a fallback name") {
    const auto invalid_code = static_cast<Error::Code>(-1);

    REQUIRE(Error::toString(invalid_code) == "Unknown");
}
