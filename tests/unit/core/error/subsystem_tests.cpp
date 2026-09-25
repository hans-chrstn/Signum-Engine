#include "engine/core/error/error_code.hpp"
#include "engine/core/error/subsystem.hpp"
#include <cstdint>
#include <gtest/gtest.h>

namespace Error = SNE::Engine::Core::Error;
constexpr std::uint8_t kUnknownSubsystemValue = 255;

TEST(SubsystemTests, GlfwErrorsBelongToPlatform) {
    EXPECT_EQ(Error::getSubsystemFor(Error::Code::GlfwInitializationFailed),
              Error::Subsystem::Platform);
}

TEST(SubsystemTests, WindowCreationErrorsBelongToPlatform) {
    EXPECT_EQ(Error::getSubsystemFor(Error::Code::WindowCreationFailed),
              Error::Subsystem::Platform);
}

TEST(SubsystemTests, VulkanRequiredExtensionErrorsBelongToVulkan) {
    EXPECT_EQ(Error::getSubsystemFor(
                  Error::Code::VulkanRequiredExtensionsUnavailable),
              Error::Subsystem::Vulkan);
}

TEST(SubsystemTests, VulkanInstanceCreationErrorsBelongToVulkan) {
    EXPECT_EQ(Error::getSubsystemFor(Error::Code::VulkanInstanceCreationFailed),
              Error::Subsystem::Vulkan);
}

TEST(SubsystemTests, VulkanLayerEnumerationErrorsBelongToVulkan) {
    EXPECT_EQ(Error::getSubsystemFor(Error::Code::VulkanLayerEnumerationFailed),
              Error::Subsystem::Vulkan);
}

TEST(SubsystemTests, VulkanValidationLayerUnavailableErrorsBelongToVulkan) {
    EXPECT_EQ(
        Error::getSubsystemFor(Error::Code::VulkanValidationLayerUnavailable),
        Error::Subsystem::Vulkan);
}

TEST(SubsystemTests,
     VulkanDebugMessengerFunctionUnavailableErrorsBelongToVulkan) {
    EXPECT_EQ(Error::getSubsystemFor(
                  Error::Code::VulkanDebugMessengerFunctionUnavailable),
              Error::Subsystem::Vulkan);
}

TEST(SubsystemTests, VulkanDebugMessengerCreationErrorsBelongToVulkan) {
    EXPECT_EQ(
        Error::getSubsystemFor(Error::Code::VulkanDebugMessengerCreationFailed),
        Error::Subsystem::Vulkan);
}

TEST(SubsystemTests, VulkanExtensionEnumerationErrorsBelongToVulkan) {
    EXPECT_EQ(
        Error::getSubsystemFor(Error::Code::VulkanExtensionEnumerationFailed),
        Error::Subsystem::Vulkan);
}

TEST(SubsystemTests, VulkanExtensionSupportUnavailableErrorsBelongToVulkan) {
    EXPECT_EQ(
        Error::getSubsystemFor(Error::Code::VulkanExtensionSupportUnavailable),
        Error::Subsystem::Vulkan);
}

TEST(SubsystemTests, VulkanApiVersionQueryErrorsBelongToVulkan) {
    EXPECT_EQ(Error::getSubsystemFor(Error::Code::VulkanApiVersionQueryFailed),
              Error::Subsystem::Vulkan);
}

TEST(SubsystemTests, VulkanApiVersionUnsupportedErrorsBelongToVulkan) {
    EXPECT_EQ(Error::getSubsystemFor(Error::Code::VulkanApiVersionUnsupported),
              Error::Subsystem::Vulkan);
}

TEST(SubsystemTests, VulkanSurfaceCreationErrorsBelongToVulkan) {
    EXPECT_EQ(Error::getSubsystemFor(Error::Code::VulkanSurfaceCreationFailed),
              Error::Subsystem::Vulkan);
}

TEST(SubsystemTests, SubsystemsHaveReadableNames) {
    EXPECT_EQ(Error::toString(Error::Subsystem::Platform), "Platform");
    EXPECT_EQ(Error::toString(Error::Subsystem::Core), "Core");
    EXPECT_EQ(Error::toString(Error::Subsystem::Editor), "Editor");
    EXPECT_EQ(Error::toString(Error::Subsystem::Renderer), "Renderer");
    EXPECT_EQ(Error::toString(Error::Subsystem::Vulkan), "Vulkan");
}

TEST(SubsystemTests, UnknownSubsystemHasFallbackName) {
    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
    EXPECT_EQ(
        Error::toString(static_cast<Error::Subsystem>(kUnknownSubsystemValue)),
        "Unknown");
}
