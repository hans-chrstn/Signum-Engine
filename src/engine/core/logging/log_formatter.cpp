#include "log_formatter.hpp"
#include "engine/core/logging/log_level.hpp"
#include <chrono>
#include <format>
#include <sstream>

namespace SNE::Engine::Core::Logging {
    auto formatLogRecord(const LogRecord &record) -> std::string {
        std::ostringstream output;

        const auto timestamp_seconds =
            std::chrono::floor<std::chrono::seconds>(record.timestamp);

        output << "[" << std::format("{:%FT%TZ}", timestamp_seconds) << "] "
               << "[" << toString(record.level) << "] " << "["
               << record.category << "] " << record.message;

        return output.str();
    }
} // namespace SNE::Engine::Core::Logging
