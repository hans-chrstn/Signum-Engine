#include "engine/core/error/error_code.hpp"
#include <cstdint>
#include <gtest/gtest.h>

namespace Error = SNE::Engine::Core::Error;
constexpr std::uint8_t kUnknownCodeValue = 255;

TEST(ErrorCodeTests, GlfwInitializationErrorCodeHasReadableName) {
    EXPECT_EQ(Error::toString(Error::Code::GlfwInitializationFailed),
              "GlfwInitializationFailed");
}

TEST(ErrorCodeTests, WindowCreationErrorCodeHasReadableName) {
    EXPECT_EQ(Error::toString(Error::Code::WindowCreationFailed),
              "WindowCreationFailed");
}

TEST(ErrorCodeTests, VulkanRequiredExtensionsErrorCodeHasReadableName) {
    EXPECT_EQ(Error::toString(Error::Code::VulkanRequiredExtensionsUnavailable),
              "VulkanRequiredExtensionsUnavailable");
}

TEST(ErrorCodeTests, VulkanInstanceCreationErrorCodeHasReadableName) {
    EXPECT_EQ(Error::toString(Error::Code::VulkanInstanceCreationFailed),
              "VulkanInstanceCreationFailed");
}

TEST(ErrorCodeTests, UnknownErrorCodeHasFallbackName) {
    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
    const auto invalid_code = static_cast<Error::Code>(kUnknownCodeValue);

    EXPECT_EQ(Error::toString(invalid_code), "Unknown");
}
