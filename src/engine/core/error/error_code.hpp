#pragma once

#include <cstdint>
#include <string_view>
namespace SNE::Engine::Core::Error {
    /**
     * @brief Identifies runtime failures recognized by the engine.
     *
     * Engine error codes describe failures at the Signum Engine abstraction
     * level. An EngineError may additionally contain native error information
     * reported by an underlying API.
     */
    enum class Code : std::uint8_t {
        /** GLFW initialization failed. */
        GlfwInitializationFailed,
        /** Creation of a platform window failed. */
        WindowCreationFailed,
        /** Required Vulkan instance extensions could not be obtained. */
        VulkanRequiredExtensionsUnavailable,
        /** Creation of the Vulkan instance failed. */
        VulkanInstanceCreationFailed,
        /** Enumeration of available Vulkan instance layers failed. */
        VulkanLayerEnumerationFailed,
        /** A Vulkan validation layer required by the engine is unavailable. */
        VulkanValidationLayerUnavailable,
        /** The Vulkan debug-messenger creation function could not be resolved.
         */
        VulkanDebugMessengerFunctionUnavailable,
        /** Creation of the Vulkan debug messenger failed. */
        VulkanDebugMessengerCreationFailed,
        /** Enumeration of available Vulkan extensions failed. */
        VulkanExtensionEnumerationFailed,
        /** A Vulkan instance extension required by the engine is unavailable.
         */
        VulkanExtensionSupportUnavailable,
        /** Query of available Vulkan instance versions failed. */
        VulkanApiVersionQueryFailed,
        /** Available Vulkan instance version is unsupported. */
        VulkanApiVersionUnsupported,
        /** Creation of the Vulkan surface failed. */
        VulkanSurfaceCreationFailed,
        /** Enumeration of physical devices available to the Vulkan instance
           failed. */
        VulkanPhysicalDeviceEnumerationFailed,
        /** Query of Vulkan surface support failed. */
        VulkanSurfaceSupportQueryFailed,
        /** Suitable Vulkan physical device could not be obtained. */
        VulkanSuitablePhysicalDeviceUnavailable,
        /** Creation of the Vulkan device failed. */
        VulkanDeviceCreationFailed,
        /** Query of available Vulkan swapchain support failed. */
        VulkanSwapchainSupportQueryFailed,
        /** Creation of the Vulkan swapchain failed. */
        VulkanSwapchainCreationFailed,
        /** Enumeration of available Vulkan images failed. */
        VulkanSwapchainImageEnumerationFailed,
        /** Creation of the Vulkan swapchain image view failed. */
        VulkanSwapchainImageViewCreationFailed,
        /** Creation of the Vulkan command pool failed. */
        VulkanCommandPoolCreationFailed,
        /** Allocation of Vulkan command buffers failed. */
        VulkanCommandBufferAllocationFailed,
        /** Creation of the Vulkan semaphore failed. */
        VulkanSemaphoreCreationFailed,
        /** Creation of a Vulkan fence failed. */
        VulkanFenceCreationFailed,
        /** Waiting for a Vulkan fence failed. */
        VulkanFenceWaitFailed,
        /** Resetting a Vulkan fence failed. */
        VulkanFenceResetFailed,
        /** Failed to acquire Vulkan swapchain image. */
        VulkanSwapchainImageAcquisitionFailed,
        /** Resetting a Vulkan command pool failed. */
        VulkanCommandPoolResetFailed,
        /** Beginning Vulkan command-buffer recording failed. */
        VulkanCommandBufferBeginFailed,
        /** Ending Vulkan command-buffer recording failed. */
        VulkanCommandBufferEndFailed,
        /** Submitting Vulkan work to a queue failed. */
        VulkanQueueSubmissionFailed,
        /** Presenting a Vulkan swapchain image failed. */
        VulkanQueuePresentationFailed,
        /** Loading compiled SPIR-V shader bytecode failed. */
        VulkanShaderBytecodeLoadFailed,
        /** Creation of a Vulkan shader module failed. */
        VulkanShaderModuleCreationFailed,
        /** Creation of a Vulkan pipeline layout failed. */
        VulkanPipelineLayoutCreationFailed,
        /** Creation of a Vulkan graphics pipeline failed. */
        VulkanGraphicsPipelineCreationFailed,
        /** Creation of a Vulkan memory allocator failed. */
        VulkanMemoryAllocatorCreationFailed,
        /** Creation of a Vulkan buffer or its backing memory allocation failed.
         */
        VulkanBufferCreationFailed,
    };

    /**
     * @brief Returns the symbolic name of an engine error code.
     *
     * @param code Engine error code to convert.
     * @return Symbolic name of the supplied error code.
     */
    [[nodiscard]] auto toString(Code code) noexcept -> std::string_view;
} // namespace SNE::Engine::Core::Error
