#include "glfw_error.hpp"
#include "engine/core/error/native_error.hpp"
#include <GLFW/glfw3.h>
#include <string>
#include <utility>

namespace SNE::Engine::Platform::Error {
    [[nodiscard]] Core::Error::NativeError captureGlfwError() {
        const char *error_description = nullptr;
        int error_code = glfwGetError(&error_description);
        std::string native_description = error_description != nullptr
                                             ? std::string(error_description)
                                             : "No description";
        return Core::Error::NativeError(error_code,
                                        std::move(native_description));
    }
} // namespace SNE::Engine::Platform::Error
