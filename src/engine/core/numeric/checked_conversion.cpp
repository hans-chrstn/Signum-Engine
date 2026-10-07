#include "checked_conversion.hpp"
#include <limits>

namespace SNE::Engine::Core::Numeric {
    auto tryConvertToUint32(std::size_t value) noexcept
        -> std::optional<std::uint32_t> {
        if (value > std::numeric_limits<std::uint32_t>::max()) {
            return std::nullopt;
        }

        return static_cast<std::uint32_t>(value);
    }
} // namespace SNE::Engine::Core::Numeric
