#pragma once

#include "engine/core/logging/log_level.hpp"

namespace SNE::Engine::Core::Logging {
    /**
     * @brief Determines whether an event meets the minimum logging severity.
     *
     * An event passes the filter when its severity is greater than
     * or equal to the configured minimum level.
     *
     * This function performs no logging, allocation, or output.
     *
     * @param level Severity of the incoming log event.
     * @param minimum_level Minimum severity permitted by the filter.
     * @return True if the event should be accepted; otherwise false.
     */
    // NOLINTBEGIN(bugprone-easily-swappable-parameters)
    [[nodiscard]] constexpr auto
    passesLevelFilter(LogLevel level, LogLevel minimum_level) noexcept -> bool {
        return level >= minimum_level;
    }
    // NOLINTEND(bugprone-easily-swappable-parameters)
} // namespace SNE::Engine::Core::Logging
