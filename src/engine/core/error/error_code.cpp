#include "error_code.hpp"

namespace SNE::Engine::Core::Error {
    auto toString(Code code) noexcept -> std::string_view {
        switch (code) {
        case Code::GlfwInitializationFailed:
            return "GlfwInitializationFailed";
        case Code::WindowCreationFailed:
            return "WindowCreationFailed";
        case Code::VulkanRequiredExtensionsUnavailable:
            return "VulkanRequiredExtensionsUnavailable";
        case Code::VulkanInstanceCreationFailed:
            return "VulkanInstanceCreationFailed";
        case Code::VulkanLayerEnumerationFailed:
            return "VulkanLayerEnumerationFailed";
        case Code::VulkanValidationLayerUnavailable:
            return "VulkanValidationLayerUnavailable";
        case Code::VulkanDebugMessengerFunctionUnavailable:
            return "VulkanDebugMessengerFunctionUnavailable";
        case Code::VulkanDebugMessengerCreationFailed:
            return "VulkanDebugMessengerCreationFailed";
        case Code::VulkanExtensionEnumerationFailed:
            return "VulkanExtensionEnumerationFailed";
        case Code::VulkanExtensionSupportUnavailable:
            return "VulkanExtensionSupportUnavailable";
        case Code::VulkanApiVersionQueryFailed:
            return "VulkanApiVersionQueryFailed";
        case Code::VulkanApiVersionUnsupported:
            return "VulkanApiVersionUnsupported";
        }
        return "Unknown";
    }
} // namespace SNE::Engine::Core::Error
