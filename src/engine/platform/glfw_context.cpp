#include "glfw_context.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include <GLFW/glfw3.h>

namespace SNE::Engine::Platform {
    GlfwContext::GlfwContext() {
        if (!glfwInit()) {
            throw Core::Error::EngineError(
                Core::Error::Code::GlfwInitializationFailed,
                "Failed to initialize GLFW!");
        }
    }

    GlfwContext::~GlfwContext() {
        glfwTerminate();
    }
} // namespace SNE::Engine::Platform
