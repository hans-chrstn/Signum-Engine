#include <cstddef>
#include <cstdint>
#include <optional>
namespace SNE::Engine::Core::Numeric {
    /**
     * @brief Attempts to convert a size value to a 32-bit unsigned integer.
     *
     * Checks whether the supplied value can be represented by std::uint32_t
     * before performing the conversion.
     *
     * This helper performs numeric validation only. The caller is responsible
     * for deciding how an unrepresentable value should be classified or
     * reported.
     *
     * @param value Value to convert.
     *
     * @return The converted value when representable; otherwise std::nullopt.
     */
    [[nodiscard]] auto tryConvertToUint32(std::size_t value) noexcept
        -> std::optional<std::uint32_t>;
} // namespace SNE::Engine::Core::Numeric
