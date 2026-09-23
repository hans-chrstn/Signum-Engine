#pragma once

#include <cstdint>
#include <string_view>
namespace SNE::Engine::Core::Error {
    enum class Code : std::uint8_t {
        GlfwInitializationFailed,
        WindowCreationFailed
    };

    [[nodiscard]] auto toString(Code code) noexcept -> std::string_view;
} // namespace SNE::Engine::Core::Error
