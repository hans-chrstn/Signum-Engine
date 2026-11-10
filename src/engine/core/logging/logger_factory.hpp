#pragma once

#include "engine/core/logging/logger.hpp"
#include "engine/core/logging/logging_configuration.hpp"

namespace SNE::Engine::Core::Logging {
    /**
     * @brief Constructs a logger using application logging policy.
     *
     * Creates and registers the logging destinations enabled by the
     * supplied configuration, then initializes the logger with the
     * configured minimum severity.
     *
     * Terminal logging uses the standard diagnostic output stream.
     * File logging uses the configured path and rotation policy.
     *
     * The returned logger owns all successfully constructed sinks.
     *
     * @param configuration Logging destinations and filtering policy.
     * @return Fully configured logger ready to receive diagnostics.
     *
     * @throws std::invalid_argument if the file rotation policy is invalid.
     * @throws Error::EngineError if file initialization fails.
     * @throws std::bad_alloc if resource allocation fails.
     *
     * @note An empty sink collection is valid.
     * @note Configuration is not modified or retained by reference.
     */
    [[nodiscard]] auto makeLogger(const LoggingConfiguration &configuration)
        -> Logger;
} // namespace SNE::Engine::Core::Logging
