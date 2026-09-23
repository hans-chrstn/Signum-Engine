#include "engine/core/error/error_code.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Error code names are readable") {
    SNE::Engine::Core::Error::Code glfw_error_code =
        SNE::Engine::Core::Error::Code::GlfwInitializationFailed;
    REQUIRE(SNE::Engine::Core::Error::toString(glfw_error_code) ==
            "GlfwInitializationFailed");

    SNE::Engine::Core::Error::Code window_error_code =
        SNE::Engine::Core::Error::Code::WindowCreationFailed;
    REQUIRE(SNE::Engine::Core::Error::toString(window_error_code) ==
            "WindowCreationFailed");
}

TEST_CASE("Unknown error codes have a fallback name") {
    const auto invalid_code = static_cast<SNE::Engine::Core::Error::Code>(-1);

    REQUIRE(SNE::Engine::Core::Error::toString(invalid_code) == "Unknown");
}
