#pragma once

#include "error_code.hpp"
#include <string_view>

namespace SNE::Engine::Core::Error {
    enum class Subsystem { Core, Platform, Vulkan, Renderer, Editor };

    [[nodiscard]] Subsystem getSubsystemFor(Code code) noexcept;
    [[nodiscard]] std::string_view toString(Subsystem subsystem) noexcept;
} // namespace SNE::Engine::Core::Error
