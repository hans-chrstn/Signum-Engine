#pragma once

#include "engine/platform/window.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include <string>

namespace SNE::Engine::Core {
    /**
     * @brief Defines the top-level configuration used to initialize an
     * application.
     *
     * Stores backend-independent application policy such as the application
     * name, initial window dimensions, preferred presentation behavior, and
     * development-diagnostic policy.
     *
     * Default values provide Signum's standard startup configuration while
     * allowing callers to override individual settings before constructing the
     * application.
     *
     * This configuration intentionally avoids Vulkan-specific options so
     * renderer implementation details remain inside the rendering backend.
     */
    struct ApplicationConfiguration {
        /** Default initial application-window width in logical window units. */
        static constexpr int kDefaultWindowWidth = 600;

        /** Default initial application-window height in logical window units.
         */
        static constexpr int kDefaultWindowHeight = 400;

        /** Application name used for the primary window and application
         * metadata. */
        std::string application_name{"Signum Editor"};

        /** Initial dimensions requested for the primary application window. */
        Platform::WindowSize initial_window_size{
            .width = kDefaultWindowWidth,
            .height = kDefaultWindowHeight,
        };

        /**
         * @brief Presentation behavior requested from the renderer.
         *
         * The rendering backend translates this backend-independent preference
         * into an appropriate native presentation configuration according to
         * available capabilities.
         */
        Renderer::PresentationPreference presentation_preference{
            Renderer::PresentationPreference::VSync};

        /**
         * @brief Whether development-oriented runtime diagnostics are enabled.
         *
         * Rendering and platform backends may use this policy to enable
         * diagnostic facilities appropriate to the active implementation.
         * Production configurations may disable these facilities when they are
         * not required.
         */
        bool development_diagnostics_enabled = true;
    };
} // namespace SNE::Engine::Core
