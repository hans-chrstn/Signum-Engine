#pragma once

#include "error_code.hpp"
#include <cstdint>
#include <string_view>

namespace SNE::Engine::Core::Error {
    enum class Subsystem : std::uint8_t {
        Core,
        Platform,
        Vulkan,
        Renderer,
        Editor
    };

    [[nodiscard]] auto getSubsystemFor(Code code) noexcept -> Subsystem;
    [[nodiscard]] auto toString(Subsystem subsystem) noexcept
        -> std::string_view;
} // namespace SNE::Engine::Core::Error
