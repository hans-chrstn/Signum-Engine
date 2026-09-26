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

TEST(ErrorCodeTests, VulkanLayerEnumerationErrorCodeHasReadableName) {
    EXPECT_EQ(Error::toString(Error::Code::VulkanLayerEnumerationFailed),
              "VulkanLayerEnumerationFailed");
}

TEST(ErrorCodeTests, VulkanValidationLayerUnavailableErrorCodeHasReadableName) {
    EXPECT_EQ(Error::toString(Error::Code::VulkanValidationLayerUnavailable),
              "VulkanValidationLayerUnavailable");
}

TEST(ErrorCodeTests,
     VulkanDebugMessengerFunctionUnavailableErrorCodeHasReadableName) {
    EXPECT_EQ(
        Error::toString(Error::Code::VulkanDebugMessengerFunctionUnavailable),
        "VulkanDebugMessengerFunctionUnavailable");
}

TEST(ErrorCodeTests, VulkanDebugMessengerCreationErrorCodeHasReadableName) {
    EXPECT_EQ(Error::toString(Error::Code::VulkanDebugMessengerCreationFailed),
              "VulkanDebugMessengerCreationFailed");
}

TEST(ErrorCodeTests, VulkanExtensionEnumerationErrorCodeHasReadableName) {
    EXPECT_EQ(Error::toString(Error::Code::VulkanExtensionEnumerationFailed),
              "VulkanExtensionEnumerationFailed");
}

TEST(ErrorCodeTests,
     VulkanExtensionSupportUnavailableErrorCodeHasReadableName) {
    EXPECT_EQ(Error::toString(Error::Code::VulkanExtensionSupportUnavailable),
              "VulkanExtensionSupportUnavailable");
}

TEST(ErrorCodeTests, VulkanApiVersionQueryErrorCodeHasReadableName) {
    EXPECT_EQ(Error::toString(Error::Code::VulkanExtensionSupportUnavailable),
              "VulkanExtensionSupportUnavailable");
}

TEST(ErrorCodeTests, VulkanApiVersionUnsupportedErrorCodeHasReadableName) {
    EXPECT_EQ(Error::toString(Error::Code::VulkanApiVersionUnsupported),
              "VulkanApiVersionUnsupported");
}

TEST(ErrorCodeTests, VulkanSurfaceCreationErrorCodeHasReadableName) {
    EXPECT_EQ(Error::toString(Error::Code::VulkanSurfaceCreationFailed),
              "VulkanSurfaceCreationFailed");
}

TEST(ErrorCodeTests, VulkanPhysicalDeviceEnumerationErrorCodeHasReadableName) {
    EXPECT_EQ(
        Error::toString(Error::Code::VulkanPhysicalDeviceEnumerationFailed),
        "VulkanPhysicalDeviceEnumerationFailed");
}

TEST(ErrorCodeTests, VulkanSurfaceSupportQueryErrorCodeHasReadableName) {
    EXPECT_EQ(Error::toString(Error::Code::VulkanSurfaceSupportQueryFailed),
              "VulkanSurfaceSupportQueryFailed");
}

TEST(ErrorCodeTests, UnknownErrorCodeHasFallbackName) {
    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
    const auto invalid_code = static_cast<Error::Code>(kUnknownCodeValue);

    EXPECT_EQ(Error::toString(invalid_code), "Unknown");
}
