#include "subsystem.hpp"
#include "error_metadata.hpp"
#include <exception>

namespace SNE::Engine::Core::Error {
    auto getSubsystemFor(Code code) noexcept -> Subsystem {
        const ErrorMetadata *metadata = findErrorMetadata(code);
        if (metadata != nullptr) {
            return metadata->subsystem;
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
