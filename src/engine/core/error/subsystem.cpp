#include "subsystem.hpp"
#include <exception>

namespace SNE::Engine::Core::Error {
    auto getSubsystemFor(Code code) noexcept -> Subsystem {
        switch (code) {
        case Code::GlfwInitializationFailed:
        case Code::WindowCreationFailed:
            return Subsystem::Platform;
        case Code::VulkanRequiredExtensionsUnavailable:
        case Code::VulkanInstanceCreationFailed:
        case Code::VulkanLayerEnumerationFailed:
        case Code::VulkanValidationLayerUnavailable:
        case Code::VulkanDebugMessengerFunctionUnavailable:
        case Code::VulkanDebugMessengerCreationFailed:
        case Code::VulkanExtensionEnumerationFailed:
        case Code::VulkanExtensionSupportUnavailable:
        case Code::VulkanApiVersionQueryFailed:
        case Code::VulkanApiVersionUnsupported:
            return Subsystem::Vulkan;
        }

        std::terminate();
    }

    auto toString(Subsystem subsystem) noexcept -> std::string_view {
        switch (subsystem) {
        case Subsystem::Platform:
            return "Platform";
        case Subsystem::Core:
            return "Core";
        case Subsystem::Editor:
            return "Editor";
        case Subsystem::Renderer:
            return "Renderer";
        case Subsystem::Vulkan:
            return "Vulkan";
        }
        return "Unknown";
    }
} // namespace SNE::Engine::Core::Error
