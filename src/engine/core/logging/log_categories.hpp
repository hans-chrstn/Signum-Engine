#pragma once

#include <string_view>

/**
 * @brief Defines standardized logging categories for engine subsystems.
 *
 * Provides stable category names for classifying diagnostic events
 * without coupling the logging system to error subsystem types.
 *
 * Categories are non-owning string views backed by string literals
 * with static storage duration.
 *
 * More specific categories may be introduced as needed.
 */
namespace SNE::Engine::Core::Logging::Categories {
    /** @brief Diagnostic events originating from core engine systems. */
    inline constexpr std::string_view Core = "Core";

    /** @brief Diagnostic events originating from platform abstractions. */
    inline constexpr std::string_view Platform = "Platform";

    /** @brief Diagnostic events originating from the Vulkan backend. */
    inline constexpr std::string_view Vulkan = "Vulkan";

    /** @brief Diagnostic events originating from rendering systems. */
    inline constexpr std::string_view Renderer = "Renderer";

    /** @brief Diagnostic events originating from editor tooling. */
    inline constexpr std::string_view Editor = "Editor";
} // namespace SNE::Engine::Core::Logging::Categories
