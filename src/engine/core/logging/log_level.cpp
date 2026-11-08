#include "log_level.hpp"
#include "engine/core/config/compiler_features.hpp"

#if SNE_HAS_CPP26_REFLECTION
    #include <meta>
#endif

namespace SNE::Engine::Core::Logging {
    auto toString(LogLevel level) noexcept -> std::string_view {
#if SNE_HAS_CPP26_REFLECTION
        template for (constexpr auto enumerator : std::define_static_array(
                          std::meta::enumerators_of(^^LogLevel))) {
            static_assert(!std::meta::identifier_of(enumerator).empty());
            if (level == [:enumerator:]) {
                return std::meta::identifier_of(enumerator);
            }
        }
#else
        switch (level) {
        case LogLevel::Trace:
            return "Trace";
        case LogLevel::Debug:
            return "Debug";
        case LogLevel::Info:
            return "Info";
        case LogLevel::Warning:
            return "Warning";
        case LogLevel::Error:
            return "Error";
        case LogLevel::Fatal:
            return "Fatal";
        }
#endif

        return "Unknown";
    }
} // namespace SNE::Engine::Core::Logging
