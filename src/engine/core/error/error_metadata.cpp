#include "error_metadata.hpp"
#include "engine/core/error/subsystem.hpp"
#include <array>
#include <cstddef>

namespace SNE::Engine::Core::Error {
    namespace {
        constexpr std::array<ErrorMetadata,
                             static_cast<std::size_t>(Code::Count)>
            error_metadata{
                ErrorMetadata{
                    .code = Code::GlfwInitializationFailed,
                    .name = "GlfwInitializationFailed",
                    .subsystem = Subsystem::Platform,
                },
                ErrorMetadata{
                    .code = Code::WindowCreationFailed,
                    .name = "WindowCreationFailed",
                    .subsystem = Subsystem::Platform,
                },
                ErrorMetadata{
                    .code = Code::VulkanRequiredExtensionsUnavailable,
                    .name = "VulkanRequiredExtensionsUnavailable",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanInstanceCreationFailed,
                    .name = "VulkanInstanceCreationFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanLayerEnumerationFailed,
                    .name = "VulkanLayerEnumerationFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanValidationLayerUnavailable,
                    .name = "VulkanValidationLayerUnavailable",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanDebugMessengerFunctionUnavailable,
                    .name = "VulkanDebugMessengerFunctionUnavailable",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanDebugMessengerCreationFailed,
                    .name = "VulkanDebugMessengerCreationFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanExtensionEnumerationFailed,
                    .name = "VulkanExtensionEnumerationFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanExtensionSupportUnavailable,
                    .name = "VulkanExtensionSupportUnavailable",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanApiVersionQueryFailed,
                    .name = "VulkanApiVersionQueryFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanApiVersionUnsupported,
                    .name = "VulkanApiVersionUnsupported",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanSurfaceCreationFailed,
                    .name = "VulkanSurfaceCreationFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanPhysicalDeviceEnumerationFailed,
                    .name = "VulkanPhysicalDeviceEnumerationFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanSurfaceSupportQueryFailed,
                    .name = "VulkanSurfaceSupportQueryFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanSuitablePhysicalDeviceUnavailable,
                    .name = "VulkanSuitablePhysicalDeviceUnavailable",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanDeviceCreationFailed,
                    .name = "VulkanDeviceCreationFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanSwapchainSupportQueryFailed,
                    .name = "VulkanSwapchainSupportQueryFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanSwapchainCreationFailed,
                    .name = "VulkanSwapchainCreationFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanSwapchainImageEnumerationFailed,
                    .name = "VulkanSwapchainImageEnumerationFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanSwapchainImageViewCreationFailed,
                    .name = "VulkanSwapchainImageViewCreationFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanCommandPoolCreationFailed,
                    .name = "VulkanCommandPoolCreationFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanCommandBufferAllocationFailed,
                    .name = "VulkanCommandBufferAllocationFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanSemaphoreCreationFailed,
                    .name = "VulkanSemaphoreCreationFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanFenceCreationFailed,
                    .name = "VulkanFenceCreationFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanFenceWaitFailed,
                    .name = "VulkanFenceWaitFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanFenceResetFailed,
                    .name = "VulkanFenceResetFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanSwapchainImageAcquisitionFailed,
                    .name = "VulkanSwapchainImageAcquisitionFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanCommandPoolResetFailed,
                    .name = "VulkanCommandPoolResetFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanCommandBufferBeginFailed,
                    .name = "VulkanCommandBufferBeginFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanCommandBufferEndFailed,
                    .name = "VulkanCommandBufferEndFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanQueueSubmissionFailed,
                    .name = "VulkanQueueSubmissionFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanQueuePresentationFailed,
                    .name = "VulkanQueuePresentationFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanShaderBytecodeLoadFailed,
                    .name = "VulkanShaderBytecodeLoadFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanShaderModuleCreationFailed,
                    .name = "VulkanShaderModuleCreationFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanPipelineLayoutCreationFailed,
                    .name = "VulkanPipelineLayoutCreationFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanGraphicsPipelineCreationFailed,
                    .name = "VulkanGraphicsPipelineCreationFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanMemoryAllocatorCreationFailed,
                    .name = "VulkanMemoryAllocatorCreationFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanBufferCreationFailed,
                    .name = "VulkanBufferCreationFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanImageCreationFailed,
                    .name = "VulkanImageCreationFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanBufferMappingFailed,
                    .name = "VulkanBufferMappingFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanBufferFlushFailed,
                    .name = "VulkanBufferFlushFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanBufferInvalidationFailed,
                    .name = "VulkanBufferInvalidationFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanDeviceWaitIdleFailed,
                    .name = "VulkanDeviceWaitIdleFailed",
                    .subsystem = Subsystem::Vulkan,
                },
                ErrorMetadata{
                    .code = Code::VulkanSpirvValidationFailed,
                    .name = "VulkanSpirvValidationFailed",
                    .subsystem = Subsystem::Vulkan,
                },
        };

        constexpr auto validateErrorMetadata() -> bool {
            for (std::size_t i{}; i < error_metadata.size(); ++i) {
                const Code code = static_cast<Code>(i);
                if (error_metadata[i].code != code) {
                    return false;
                }
            }

            return true;
        }

        static_assert(validateErrorMetadata(),
                      "Error metadata must contain exactly one correctly "
                      "ordered entry for every error code");
    } // namespace

    auto findErrorMetadata(Code code) noexcept -> const ErrorMetadata * {
        const auto index = static_cast<std::size_t>(code);

        if (index >= error_metadata.size()) {
            return nullptr;
        }

        if (error_metadata[index].code != code) {
            return nullptr;
        }

        return &error_metadata[index];
    }
} // namespace SNE::Engine::Core::Error
