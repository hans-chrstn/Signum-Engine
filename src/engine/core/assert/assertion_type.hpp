#pragma once

#include <cstdint>
#include <string_view>

namespace SNE::Engine::Core::Assertion {
    /**
     * @brief Identifies the category of a failed engine assertion.
     *
     * Assertion types describe programming-contract failures detected by the
     * engine. They distinguish invalid caller input from broken internal state
     * and failed guarantees established by an operation.
     */
    enum class AssertionType : std::uint8_t {
        /** A caller violated a requirement of an operation. */
        Precondition,
        /** An operation failed to establish a state it guarantees on
           completion. */
        Postcondition,
        /** An internal engine state violated an assumption that must always
           hold. */
        Invariant,

    };

    /**
     * @brief Returns the readable name of an assertion type.
     *
     * @param type Assertion type to convert.
     * @return Readable name of the supplied assertion type.
     */
    [[nodiscard]] auto toString(AssertionType type) noexcept
        -> std::string_view;
} // namespace SNE::Engine::Core::Assertion
