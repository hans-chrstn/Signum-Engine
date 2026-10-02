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
    EXPECT_EQ(Error::toString(Error::Code::VulkanApiVersionQueryFailed),
              "VulkanApiVersionQueryFailed");
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

TEST(ErrorCodeTests, VulkanSuitablePhysicalDeviceErrorCodeHasReadableName) {
    EXPECT_EQ(
        Error::toString(Error::Code::VulkanSuitablePhysicalDeviceUnavailable),
        "VulkanSuitablePhysicalDeviceUnavailable");
}

TEST(ErrorCodeTests, VulkanDeviceCreationErrorCodeHasReadableName) {
    EXPECT_EQ(Error::toString(Error::Code::VulkanDeviceCreationFailed),
              "VulkanDeviceCreationFailed");
}

TEST(ErrorCodeTests, VulkanSwapchainSupportQueryErrorCodeHasReadableName) {
    EXPECT_EQ(Error::toString(Error::Code::VulkanSwapchainSupportQueryFailed),
              "VulkanSwapchainSupportQueryFailed");
}

TEST(ErrorCodeTests, VulkanSwapchainCreationErrorCodeHasReadableName) {
    EXPECT_EQ(Error::toString(Error::Code::VulkanSwapchainCreationFailed),
              "VulkanSwapchainCreationFailed");
}

TEST(ErrorCodeTests, VulkanSwapchainImageEnumerationErrorCodeHasReadableName) {
    EXPECT_EQ(
        Error::toString(Error::Code::VulkanSwapchainImageEnumerationFailed),
        "VulkanSwapchainImageEnumerationFailed");
}

TEST(ErrorCodeTests, VulkanSwapchainImageViewCreationErrorCodeHasReadableName) {
    EXPECT_EQ(
        Error::toString(Error::Code::VulkanSwapchainImageViewCreationFailed),
        "VulkanSwapchainImageViewCreationFailed");
}

TEST(ErrorCodeTests, VulkanCommandPoolCreationErrorCodeHasReadableName) {
    EXPECT_EQ(Error::toString(Error::Code::VulkanCommandPoolCreationFailed),
              "VulkanCommandPoolCreationFailed");
}

TEST(ErrorCodeTests, VulkanCommandBufferAllocationErrorCodeHasReadableName) {
    EXPECT_EQ(Error::toString(Error::Code::VulkanCommandBufferAllocationFailed),
              "VulkanCommandBufferAllocationFailed");
}

TEST(ErrorCodeTests, VulkanSemaphoreCreationErrorCodeHasReadableName) {
    EXPECT_EQ(Error::toString(Error::Code::VulkanSemaphoreCreationFailed),
              "VulkanSemaphoreCreationFailed");
}

TEST(ErrorCodeTests, VulkanFenceCreationErrorCodeHasReadableName) {
    EXPECT_EQ(Error::toString(Error::Code::VulkanFenceCreationFailed),
              "VulkanFenceCreationFailed");
}

TEST(ErrorCodeTests, VulkanFenceWaitErrorCodeHasReadableName) {
    EXPECT_EQ(Error::toString(Error::Code::VulkanFenceWaitFailed),
              "VulkanFenceWaitFailed");
}

TEST(ErrorCodeTests, VulkanFenceResetErrorCodeHasReadableName) {
    EXPECT_EQ(Error::toString(Error::Code::VulkanFenceResetFailed),
              "VulkanFenceResetFailed");
}

TEST(ErrorCodeTests, UnknownErrorCodeHasFallbackName) {
    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
    const auto invalid_code = static_cast<Error::Code>(kUnknownCodeValue);

    EXPECT_EQ(Error::toString(invalid_code), "Unknown");
}
