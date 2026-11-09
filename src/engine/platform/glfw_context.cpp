#include "glfw_context.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/core/logging/log_level.hpp"
#include "engine/core/logging/logger.hpp"
#include "error/glfw_error.hpp"
#include <GLFW/glfw3.h>
#include <cstdio>
#include <format>

namespace SNE::Engine::Platform {
    GlfwContext::GlfwContext(Core::Logging::Logger *logger) {

        if (glfwInit() == 0) {
            Core::Error::NativeError native_error = Error::captureGlfwError();

            if (logger != nullptr) {
                try {
                    logger->log(
                        Core::Logging::LogLevel::Error, "Platform",
                        std::format("GLFW initialization failed: {}",
                                    native_error.getNativeDescription()));
                } catch (...) {
                    std::fputs("Warning: Failed to log GLFW error.\n", stderr);
                }
            }

            throw Core::Error::EngineError(
                Core::Error::Code::GlfwInitializationFailed,
                "Failed to initialize GLFW!", native_error, "Initialize GLFW");
        }

        if (logger == nullptr) {
            return;
        }

        try {
            logger->log(Core::Logging::LogLevel::Info, "Platform",
                        "GLFW initialized");
        } catch (...) {
            std::fputs(
                "Warning: Failed to log GLFW initialization diagnostic.\n",
                stderr);
        }
    }

    GlfwContext::~GlfwContext() {
        glfwTerminate();
    }
} // namespace SNE::Engine::Platform
