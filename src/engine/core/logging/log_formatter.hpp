#pragma once

#include "engine/core/logging/log_record.hpp"
#include <string>

namespace SNE::Engine::Core::Logging {
    /**
     * @brief Formats a structured log record as human-readable text.
     *
     * Produces a consistent representation containing the event timestamp,
     * severity, category, and message.
     *
     * This function performs no output and does not modify the record.
     * The returned string owns its formatted contents independently
     * of the source record.
     *
     * @param record Structured logging event to format.
     * @return Human-readable representation of the logging event.
     *
     * @throws std::bad_alloc if string allocation fails.
     */
    [[nodiscard]] auto formatLogRecord(const LogRecord &record) -> std::string;
} // namespace SNE::Engine::Core::Logging
