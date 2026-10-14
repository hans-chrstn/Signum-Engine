#pragma once

#include "engine/platform/window.hpp"
#include "engine/renderer/camera/initial_camera_data.hpp"
#include "engine/renderer/geometry/initial_mesh_data.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/shader/graphics_shader_data.hpp"
#include "engine/renderer/texture/texture_upload_data.hpp"
#include <optional>
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
            Renderer::PresentationPreference::VSync,
        };

        /**
         * @brief Whether development-oriented runtime diagnostics are enabled.
         *
         * Rendering and platform backends may use this policy to enable
         * diagnostic facilities appropriate to the active implementation.
         * Production configurations may disable these facilities when they are
         * not required.
         */
        bool development_diagnostics_enabled = true;

        /**
         * @brief Optional initial geometry supplied to the renderer.
         *
         * Contains non-owning views of CPU mesh data and vertex attributes.
         * The referenced storage must remain valid until renderer
         * initialization completes.
         *
         * When absent, no application-provided geometry is requested.
         */
        std::optional<Renderer::InitialMeshData> initial_mesh;

        /**
         * @brief Optional initial texture supplied to the renderer.
         *
         * Contains the dimensions and borrowed RGBA8 pixel data
         * used to initialize a GPU texture.
         *
         * The referenced pixel storage must remain valid until the
         * renderer has finished consuming it during initialization.
         *
         * When absent, no initial texture is requested.
         */
        std::optional<Renderer::TextureUploadData> initial_texture;

        /**
         * @brief Optional initial camera configuration supplied to the
         * renderer.
         *
         * Stores the initial world-to-camera view transformation and
         * perspective projection parameters.
         *
         * The configuration is copied during renderer initialization,
         * allowing the renderer to retain camera information without
         * borrowing application-managed storage.
         *
         * The rendering backend derives the projection aspect ratio
         * from the current drawable framebuffer dimensions.
         *
         * When absent, no application-provided camera is configured.
         */
        std::optional<Renderer::InitialCameraData> initial_camera;

        /**
         * @brief Optional initial graphics shaders supplied to the renderer.
         *
         * Owns the compiled SPIR-V bytecode and entry-point selections
         * for the vertex and fragment shader stages.
         *
         * The renderer may take ownership of these shaders during
         * initialization and retain them for graphics-pipeline recreation.
         *
         * When absent, no application-provided graphics shaders are configured.
         */
        std::optional<Renderer::Shader::GraphicsShaderData>
            initial_graphics_shaders;
    };
} // namespace SNE::Engine::Core
