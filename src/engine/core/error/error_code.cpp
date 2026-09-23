#include "error_code.hpp"

namespace SNE::Engine::Core::Error {
    std::string_view toString(Code code) noexcept {
        switch (code) {
        case Code::GlfwInitializationFailed:
            return "GlfwInitializationFailed";
        case Code::WindowCreationFailed:
            return "WindowCreationFailed";
        }
        return "Unknown";
    }
} // namespace SNE::Engine::Core::Error
