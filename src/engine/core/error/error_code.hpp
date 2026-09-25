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
        /** Enumeration of available Vulkan instance extensions failed. */
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
        /** Enumeration of available Vulkan instance physical devices failed. */
        VulkanPhysicalDeviceEnumerationFailed,
    };

    /**
     * @brief Returns the symbolic name of an engine error code.
     *
     * @param code Engine error code to convert.
     * @return Symbolic name of the supplied error code.
     */
    [[nodiscard]] auto toString(Code code) noexcept -> std::string_view;
} // namespace SNE::Engine::Core::Error
