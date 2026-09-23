#pragma once

#include <string_view>
namespace SNE::Engine::Core::Error {
    enum class Code { GlfwInitializationFailed, WindowCreationFailed };

    [[nodiscard]] std::string_view toString(Code code) noexcept;
} // namespace SNE::Engine::Core::Error
