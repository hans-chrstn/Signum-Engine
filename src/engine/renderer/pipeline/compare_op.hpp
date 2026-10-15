#pragma once

#include <cstdint>

namespace SNE::Engine::Renderer {
    /**
     * @brief Specifies the comparison operation used by depth testing.
     *
     * Determines whether a fragment passes a depth comparison
     * against the value stored in the depth attachment.
     */
    enum class CompareOp : std::uint8_t {
        /** @brief The comparison always fails. */
        Never,

        /** @brief Passes when the incoming value is less than the stored value.
         */
        Less,

        /** @brief Passes when the incoming value equals the stored value. */
        Equal,

        /** @brief Passes when the incoming value is less than or equal to the
           stored value. */
        LessOrEqual,

        /** @brief Passes when the incoming value is greater than the stored
           value. */
        Greater,

        /** @brief Passes when the incoming value differs from the stored value.
         */
        NotEqual,

        /** @brief Passes when the incoming value is greater than or equal to
           the stored value. */
        GreaterOrEqual,

        /** @brief The comparison always passes. */
        Always,
    };
} // namespace SNE::Engine::Renderer
