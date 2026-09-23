#pragma once

#include "error_code.hpp"

namespace SNE::Engine::Core::Error {
    enum class Subsystem { Core, Platform, Vulkan, Renderer, Editor };

    [[nodiscard]] Subsystem getSubsystemFor(Code code) noexcept;
} // namespace SNE::Engine::Core::Error
