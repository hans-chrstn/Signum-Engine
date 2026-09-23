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
} // namespace SNE::Engine::Core::Error
