#include "glfw_context.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/core/logging/log_level.hpp"
#include "engine/core/logging/logger.hpp"
#include "error/glfw_error.hpp"
#include <GLFW/glfw3.h>
#include <condition_variable>
#include <cstddef>
#include <cstdio>
#include <format>
#include <mutex>
#include <optional>
#include <utility>

namespace SNE::Engine::Platform {
    namespace {
        struct GlfwErrorCallbackState {
            std::mutex mutex;
            std::condition_variable condition_variable;
            Core::Logging::Logger *logger{nullptr};
            std::size_t count{0};
            bool accepted{false};
            bool context_active{false};
        };

        class LoggerLease {
          private:
            Core::Logging::Logger *m_Logger{nullptr};
            GlfwErrorCallbackState *m_State{nullptr};

            explicit LoggerLease(Core::Logging::Logger *logger,
                                 GlfwErrorCallbackState *state)
                : m_Logger(logger), m_State(state) {}

          public:
            LoggerLease(const LoggerLease &) = delete;
            auto operator=(const LoggerLease &) -> LoggerLease & = delete;

            LoggerLease(LoggerLease &&other) noexcept
                : m_Logger(std::exchange(other.m_Logger, nullptr)),
                  m_State(std::exchange(other.m_State, nullptr)) {}

            auto operator=(LoggerLease &&other) -> LoggerLease & = delete;

            ~LoggerLease() noexcept {
                if (m_State != nullptr) {
                    bool count_zero{false};

                    {
                        std::scoped_lock<std::mutex> lock{m_State->mutex};
                        --m_State->count;
                        count_zero = m_State->count == 0U;
                    }

                    if (count_zero) {
                        m_State->condition_variable.notify_all();
                    }
                }
            }

            friend auto tryAcquireLogger() -> std::optional<LoggerLease>;

            [[nodiscard]] auto logger() const noexcept
                -> Core::Logging::Logger & {
                return *m_Logger;
            }
        };

        [[nodiscard]] auto glfwErrorCallbackState()
            -> GlfwErrorCallbackState & {
            static GlfwErrorCallbackState state;
            return state;
        }

        [[nodiscard]] auto tryAcquireLogger() -> std::optional<LoggerLease> {
            GlfwErrorCallbackState &state = glfwErrorCallbackState();

            std::scoped_lock<std::mutex> lock{state.mutex};

            if (!state.accepted || state.logger == nullptr) {
                return std::nullopt;
            }

            ++state.count;

            return LoggerLease{state.logger, &state};
        }

        auto glfwErrorCallback(int error_code, const char *description) noexcept
            -> void {
            try {
                std::optional<LoggerLease> lease = tryAcquireLogger();

                if (!lease.has_value()) {
                    return;
                }

                auto &logger = lease->logger();

                logger.log(Core::Logging::LogLevel::Error, "Platform",
                           std::format("GLFW error {}: {}", error_code,
                                       description != nullptr
                                           ? description
                                           : "No description"));

            } catch (...) {
                std::fputs("Warning: Failed to log GLFW error.\n", stderr);
            }
        }

        [[nodiscard]] auto tryClaimGlfwContext() -> bool {
            GlfwErrorCallbackState &state = glfwErrorCallbackState();

            std::scoped_lock<std::mutex> lock{state.mutex};

            if (!state.context_active) {
                state.context_active = true;
                return true;
            }

            return false;
        }

        auto connectGlfwLogger(Core::Logging::Logger *logger) -> void {
            GlfwErrorCallbackState &state = glfwErrorCallbackState();

            std::scoped_lock<std::mutex> lock{state.mutex};

            state.logger = logger;

            state.accepted = logger != nullptr;
        }

        auto disconnectGlfwLogger() -> void {
            GlfwErrorCallbackState &state = glfwErrorCallbackState();

            std::unique_lock<std::mutex> lock{state.mutex};

            state.accepted = false;

            state.condition_variable.wait(
                lock, [&state] -> bool { return state.count == 0U; });

            state.logger = nullptr;
        }

        auto releaseGlfwContext() -> void {
            GlfwErrorCallbackState &state = glfwErrorCallbackState();

            std::scoped_lock<std::mutex> lock{state.mutex};

            state.context_active = false;
        }
    } // namespace

    GlfwContext::GlfwContext(Core::Logging::Logger *logger) {

        if (!tryClaimGlfwContext()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Platform,
                "GlfwContext requires exclusive ownership of the process-wide "
                "GLFW initialization lifetime");
        }

        try {
            connectGlfwLogger(logger);

            glfwSetErrorCallback(glfwErrorCallback);

            if (glfwInit() == 0) {
                Core::Error::NativeError native_error =
                    Error::captureGlfwError();

                if (logger != nullptr) {
                    try {
                        logger->log(
                            Core::Logging::LogLevel::Error, "Platform",
                            std::format("GLFW initialization failed: {}",
                                        native_error.getNativeDescription()));
                    } catch (...) {
                        std::fputs("Warning: Failed to log GLFW error.\n",
                                   stderr);
                    }
                }

                throw Core::Error::EngineError(
                    Core::Error::Code::GlfwInitializationFailed,
                    "Failed to initialize GLFW!", native_error,
                    "Initialize GLFW");
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
        } catch (...) {
            glfwSetErrorCallback(nullptr);
            disconnectGlfwLogger();
            releaseGlfwContext();
            throw;
        }
    }

    GlfwContext::~GlfwContext() {
        glfwTerminate();
        glfwSetErrorCallback(nullptr);
        disconnectGlfwLogger();
        releaseGlfwContext();
    }
} // namespace SNE::Engine::Platform
