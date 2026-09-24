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
        }
        return "Unknown";
    }
} // namespace SNE::Engine::Core::Error
