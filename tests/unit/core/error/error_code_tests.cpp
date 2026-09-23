#include "engine/core/error/error_code.hpp"
#include <catch2/catch_test_macros.hpp>

namespace Error = SNE::Engine::Core::Error;
constexpr std::uint8_t kUnknownCodeValue = 255;

TEST_CASE("Error code names are readable") {
    Error::Code glfw_error_code = Error::Code::GlfwInitializationFailed;
    REQUIRE(Error::toString(glfw_error_code) == "GlfwInitializationFailed");

    Error::Code window_error_code = Error::Code::WindowCreationFailed;
    REQUIRE(Error::toString(window_error_code) == "WindowCreationFailed");
}

TEST_CASE("Unknown error codes have a fallback name") {
    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
    const auto invalid_code = static_cast<Error::Code>(kUnknownCodeValue);

    REQUIRE(Error::toString(invalid_code) == "Unknown");
}
