#include "engine/core/logging/log_level_filter.hpp"

#include <array>
#include <cstddef>
#include <gtest/gtest.h>

namespace Logging = SNE::Engine::Core::Logging;

namespace {
    constexpr std::array kOrderedLevels{
        Logging::LogLevel::Trace, Logging::LogLevel::Debug,
        Logging::LogLevel::Info,  Logging::LogLevel::Warning,
        Logging::LogLevel::Error, Logging::LogLevel::Fatal,
    };

    static_assert(Logging::passesLevelFilter(Logging::LogLevel::Warning,
                                             Logging::LogLevel::Warning));

    static_assert(Logging::passesLevelFilter(Logging::LogLevel::Fatal,
                                             Logging::LogLevel::Warning));

    static_assert(!Logging::passesLevelFilter(Logging::LogLevel::Debug,
                                              Logging::LogLevel::Warning));
} // namespace

TEST(LogLevelFilterTests, FiltersAllSeverityCombinations) {
    for (std::size_t level_index = 0; level_index < kOrderedLevels.size();
         ++level_index) {
        for (std::size_t minimum_index = 0;
             minimum_index < kOrderedLevels.size(); ++minimum_index) {
            const bool expected = level_index >= minimum_index;

            EXPECT_EQ(Logging::passesLevelFilter(kOrderedLevels[level_index],
                                                 kOrderedLevels[minimum_index]),
                      expected)
                << "Event severity index: " << level_index
                << ", minimum severity index: " << minimum_index;
        }
    }
}
