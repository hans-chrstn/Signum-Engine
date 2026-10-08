#pragma once

#include "engine/core/error/error_code.hpp"
#include "engine/core/error/subsystem.hpp"
#include <string_view>
namespace SNE::Engine::Core::Error {
    /**
     * @brief Describes static metadata associated with an engine error code.
     *
     * Associates an engine error code with its symbolic name and owning
     * subsystem.
     *
     * Error metadata is immutable engine-defined information used by diagnostic
     * and classification helpers. The structure does not own any resources.
     */
    struct ErrorMetadata {
        /** Engine error code described by this metadata entry. */
        Code code;

        /** Symbolic name of the engine error code. */
        std::string_view name;

        /** Engine subsystem associated with the error code. */
        Subsystem subsystem;
    };

    /**
     * @brief Finds metadata associated with an engine error code.
     *
     * Looks up the static metadata entry corresponding to the supplied engine
     * error code.
     *
     * The returned pointer is non-owning and refers to immutable engine-defined
     * metadata with static storage duration.
     *
     * @param code Engine error code whose metadata is requested.
     *
     * @return Pointer to the associated metadata when code identifies a valid
     * reportable engine error; otherwise nullptr.
     *
     * @note Code::Count is a sentinel and does not have an associated metadata
     * entry.
     */
    [[nodiscard]] auto findErrorMetadata(Code code) noexcept
        -> const ErrorMetadata *;
} // namespace SNE::Engine::Core::Error
