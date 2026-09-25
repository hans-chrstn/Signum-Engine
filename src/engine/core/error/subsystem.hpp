#pragma once

#include "error_code.hpp"
#include <cstdint>
#include <string_view>

namespace SNE::Engine::Core::Error {
    /**
     * @brief Identifies a logical subsystem of the engine.
     *
     * Subsystems are used to categorize engine errors and diagnostics according
     * to the component responsible for the failure.
     */
    enum class Subsystem : std::uint8_t {
        Core,
        Platform,
        Vulkan,
        Renderer,
        Editor
    };

    /**
     * @brief Determines the subsystem associated with an engine error code.
     *
     * @param code Engine error code to classify.
     * @return Subsystem responsible for the supplied error code.
     */
    [[nodiscard]] auto getSubsystemFor(Code code) noexcept -> Subsystem;

    /**
     * @brief Returns the symbolic name of an engine subsystem.
     *
     * @param subsystem Subsystem to convert.
     * @return Symbolic name of the supplied subsystem.
     */
    [[nodiscard]] auto toString(Subsystem subsystem) noexcept
        -> std::string_view;
} // namespace SNE::Engine::Core::Error
