#pragma once

#include "engine/platform/window.hpp"
#include "engine/renderer/camera/initial_camera_data.hpp"
#include "engine/renderer/geometry/initial_mesh_data.hpp"
#include "engine/renderer/graphics_backend.hpp"
#include "engine/renderer/material/material_buffer_binding_input.hpp"
#include "engine/renderer/material/material_texture_binding_input.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/shader/graphics_shader_data.hpp"
#include "engine/renderer/texture/texture_upload_data.hpp"
#include <optional>
#include <span>
#include <string>

namespace SNE::Engine::Core::Logging {
    class Logger;
}

namespace SNE::Engine::Renderer {
    /**
     * @brief Describes the resources and configuration required to create
     * a renderer.
     *
     * Carries backend-independent initialization settings used by the
     * renderer factory to construct the selected graphics implementation.
     *
     * Owns configuration values and optional shader artifacts while
     * borrowing the application window, logger, and externally supplied
     * initialization data.
     *
     * Borrowed data must remain valid until renderer construction
     * finishes. The application window and logger must outlive the
     * constructed renderer.
     *
     * @note This structure does not own GPU resources or create a
     * rendering device.
     *
     * @note The selected graphics backend cannot be changed after
     * the renderer has been constructed.
     */
    struct RendererCreationInfo {

        /** @brief Graphics backend requested for renderer creation. */
        GraphicsBackend backend{GraphicsBackend::Vulkan};

        /** @brief Application name supplied to the graphics backend. */
        std::string application_name;

        /**
         * @brief Borrowed application window used for renderer initialization.
         *
         * The window must outlive the constructed renderer.
         */
        const Platform::Window &window;

        /**
         * @brief Requested presentation behavior.
         *
         * The backend selects a compatible native presentation configuration.
         */
        PresentationPreference presentation_preference{
            PresentationPreference::VSync,
        };

        /**
         * @brief Whether development-oriented graphics diagnostics are enabled.
         */
        bool development_diagnostics_enabled{true};

        /**
         * @brief Optional initial mesh supplied to the renderer.
         *
         * Mesh data and vertex attributes are borrowed until renderer
         * initialization completes.
         */
        std::optional<InitialMeshData> initial_mesh = std::nullopt;

        /**
         * @brief Optional initial texture supplied to the renderer.
         *
         * Referenced pixel storage must remain valid until the renderer
         * completes its initial texture upload.
         */
        std::optional<TextureUploadData> initial_texture = std::nullopt;

        /**
         * @brief Optional initial camera configuration.
         *
         * Contains the initial camera transformation and projection
         * parameters used by the renderer.
         */
        std::optional<InitialCameraData> initial_camera = std::nullopt;

        /**
         * @brief Optional initial graphics shader configuration.
         *
         * Owns compiled shader artifacts and entry-point selections that
         * may be transferred to the constructed renderer.
         */
        std::optional<Shader::GraphicsShaderData> initial_graphics_shaders =
            std::nullopt;

        /**
         * @brief Initial material buffer bindings supplied to the renderer.
         *
         * Binding descriptions and referenced CPU data are borrowed until
         * renderer initialization completes.
         *
         * An empty span indicates that no initial buffers are supplied.
         */
        std::span<const MaterialBufferBindingInput> initial_buffer_bindings;

        /**
         * @brief Initial sampled-texture bindings supplied to the renderer.
         *
         * Binding descriptions and referenced pixel storage are borrowed
         * until renderer initialization completes.
         *
         * An empty span indicates that no initial textures are supplied.
         */
        std::span<const MaterialTextureBindingInput> initial_texture_bindings;

        /**
         * @brief Whether presentation images require transfer-source support.
         *
         * Allows renderer initialization to request swapchain image
         * capabilities needed for operations such as framebuffer readback.
         */
        bool require_swapchain_transfer_source{false};

        /**
         * @brief Optional borrowed logger for renderer diagnostics.
         *
         * A null pointer disables engine-backed diagnostic forwarding.
         * A non-null logger must outlive the constructed renderer and all
         * diagnostic callbacks referencing it.
         */
        Core::Logging::Logger *logger{nullptr};
    };
} // namespace SNE::Engine::Renderer
