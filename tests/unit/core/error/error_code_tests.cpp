#include "engine/core/error/error_code.hpp"
#include "engine/core/error/subsystem.hpp"
#include <cstdint>
#include <gtest/gtest.h>
#include <ostream>
#include <string_view>

namespace Error = SNE::Engine::Core::Error;

namespace {
    struct ErrorCodeTestCase {
        Error::Code code;
        std::string_view expected_name;
        Error::Subsystem expected_subsystem;
    };

    [[maybe_unused]] auto PrintTo(const ErrorCodeTestCase &test_case,
                                  std::ostream *stream) -> void {
        *stream << "{ code = " << test_case.expected_name
                << ", expected_name = " << test_case.expected_name
                << ", expected_subsystem = "
                << Error::toString(test_case.expected_subsystem) << " }";
    }

    class ErrorCodeTests : public testing::TestWithParam<ErrorCodeTestCase> {};

    constexpr std::uint8_t kUnknownCodeValue = 255;
} // namespace

TEST_P(ErrorCodeTests, HasReadableName) {
    const ErrorCodeTestCase &test_case = GetParam();

    EXPECT_EQ(Error::toString(test_case.code), test_case.expected_name);
}

TEST_P(ErrorCodeTests, BelongsToExpectedSubsystem) {
    const ErrorCodeTestCase &test_case = GetParam();

    EXPECT_EQ(Error::getSubsystemFor(test_case.code),
              test_case.expected_subsystem);
}

INSTANTIATE_TEST_SUITE_P(
    KnownErrorCodes, ErrorCodeTests,
    ::testing::Values(
        ErrorCodeTestCase{
            Error::Code::GlfwInitializationFailed,
            "GlfwInitializationFailed",
            Error::Subsystem::Platform,
        },
        ErrorCodeTestCase{
            Error::Code::WindowCreationFailed,
            "WindowCreationFailed",
            Error::Subsystem::Platform,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanRequiredExtensionsUnavailable,
            "VulkanRequiredExtensionsUnavailable",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanInstanceCreationFailed,
            "VulkanInstanceCreationFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanLayerEnumerationFailed,
            "VulkanLayerEnumerationFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanValidationLayerUnavailable,
            "VulkanValidationLayerUnavailable",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanDebugMessengerFunctionUnavailable,
            "VulkanDebugMessengerFunctionUnavailable",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanDebugMessengerCreationFailed,
            "VulkanDebugMessengerCreationFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanExtensionEnumerationFailed,
            "VulkanExtensionEnumerationFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanExtensionSupportUnavailable,
            "VulkanExtensionSupportUnavailable",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanApiVersionQueryFailed,
            "VulkanApiVersionQueryFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanApiVersionUnsupported,
            "VulkanApiVersionUnsupported",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanSurfaceCreationFailed,
            "VulkanSurfaceCreationFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanPhysicalDeviceEnumerationFailed,
            "VulkanPhysicalDeviceEnumerationFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanSurfaceSupportQueryFailed,
            "VulkanSurfaceSupportQueryFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanSuitablePhysicalDeviceUnavailable,
            "VulkanSuitablePhysicalDeviceUnavailable",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanDeviceCreationFailed,
            "VulkanDeviceCreationFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanSwapchainSupportQueryFailed,
            "VulkanSwapchainSupportQueryFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanSwapchainCreationFailed,
            "VulkanSwapchainCreationFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanSwapchainImageEnumerationFailed,
            "VulkanSwapchainImageEnumerationFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanSwapchainImageViewCreationFailed,
            "VulkanSwapchainImageViewCreationFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanCommandPoolCreationFailed,
            "VulkanCommandPoolCreationFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanCommandBufferAllocationFailed,
            "VulkanCommandBufferAllocationFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanSemaphoreCreationFailed,
            "VulkanSemaphoreCreationFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanFenceCreationFailed,
            "VulkanFenceCreationFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanFenceWaitFailed,
            "VulkanFenceWaitFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanFenceResetFailed,
            "VulkanFenceResetFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanSwapchainImageAcquisitionFailed,
            "VulkanSwapchainImageAcquisitionFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanCommandPoolResetFailed,
            "VulkanCommandPoolResetFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanCommandBufferBeginFailed,
            "VulkanCommandBufferBeginFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanCommandBufferEndFailed,
            "VulkanCommandBufferEndFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanQueueSubmissionFailed,
            "VulkanQueueSubmissionFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanQueuePresentationFailed,
            "VulkanQueuePresentationFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanShaderBytecodeLoadFailed,
            "VulkanShaderBytecodeLoadFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanShaderModuleCreationFailed,
            "VulkanShaderModuleCreationFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanPipelineLayoutCreationFailed,
            "VulkanPipelineLayoutCreationFailed",
            Error::Subsystem::Vulkan,
        },
        ErrorCodeTestCase{
            Error::Code::VulkanGraphicsPipelineCreationFailed,
            "VulkanGraphicsPipelineCreationFailed",
            Error::Subsystem::Vulkan,
        }));

TEST(ErrorCodeFallbackTests, UnknownErrorCodeHasFallbackName) {
    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
    const auto invalid_code = static_cast<Error::Code>(kUnknownCodeValue);

    EXPECT_EQ(Error::toString(invalid_code), "Unknown");
}
