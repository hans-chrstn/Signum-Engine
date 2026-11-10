#include "logger_factory.hpp"
#include "engine/core/logging/file_log_sink.hpp"
#include "engine/core/logging/log_sink.hpp"
#include "engine/core/logging/stream_log_sink.hpp"
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

namespace SNE::Engine::Core::Logging {
    auto makeLogger(const LoggingConfiguration &configuration) -> Logger {
        std::vector<std::unique_ptr<LogSink>> sinks;

        if (configuration.terminal_enabled) {
            sinks.push_back(std::make_unique<StreamLogSink>(std::clog));
        }

        if (configuration.file_path.has_value()) {
            sinks.push_back(std::make_unique<FileLogSink>(
                configuration.file_path.value(),
                configuration.file_rotation_policy));
        }

        return Logger{std::move(sinks), configuration.minimum_level};
    }
} // namespace SNE::Engine::Core::Logging
