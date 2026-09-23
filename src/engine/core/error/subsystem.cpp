#include "subsystem.hpp"
#include <exception>

namespace SNE::Engine::Core::Error {
    Subsystem getSubsystemFor(Code code) noexcept {
        switch (code) {
        case Code::GlfwInitializationFailed:
        case Code::WindowCreationFailed:
            return Subsystem::Platform;
        }

        std::terminate();
    }

    std::string_view toString(Subsystem subsystem) noexcept {
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
