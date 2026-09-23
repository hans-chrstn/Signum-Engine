#include "error_code.hpp"

namespace SNE::Engine::Core::Error {
    auto toString(Code code) noexcept -> std::string_view {
        switch (code) {
        case Code::GlfwInitializationFailed:
            return "GlfwInitializationFailed";
        case Code::WindowCreationFailed:
            return "WindowCreationFailed";
        }
        return "Unknown";
    }
} // namespace SNE::Engine::Core::Error
