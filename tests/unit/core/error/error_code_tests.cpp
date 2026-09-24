#include "engine/core/error/error_code.hpp"
#include <cstdint>
#include <gtest/gtest.h>

namespace Error = SNE::Engine::Core::Error;
constexpr std::uint8_t kUnknownCodeValue = 255;

TEST(ErrorCodeTests, ErrorCodesHaveReadableNames) {
    Error::Code glfw_error_code = Error::Code::GlfwInitializationFailed;
    EXPECT_EQ(Error::toString(glfw_error_code), "GlfwInitializationFailed");

    Error::Code window_error_code = Error::Code::WindowCreationFailed;
    EXPECT_EQ(Error::toString(window_error_code), "WindowCreationFailed");
}

TEST(ErrorCodeTests, UnknownErrorCodeHasFallbackName) {
    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
    const auto invalid_code = static_cast<Error::Code>(kUnknownCodeValue);

    EXPECT_EQ(Error::toString(invalid_code), "Unknown");
}
