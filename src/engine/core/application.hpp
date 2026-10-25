#pragma once

#include "application_configuration.hpp"
#include "engine/core/math/matrix4.hpp"
#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/material/material_buffer_update_input.hpp"
#include "engine/renderer/shader/shader_stage_input.hpp"
#include "engine/renderer/vulkan/vulkan_renderer.hpp"
#include <cstddef>
#include <functional>

namespace SNE::Engine::Core {
    /**
     * @brief Owns and coordinates the top-level engine lifetime.
     *
     * Establishes the lifetime of engine-wide configuration, platform, window,
     * and rendering systems. Members are declared in dependency order so
     * construction occurs from lower-level dependencies to higher-level systems
     * and destruction occurs safely in reverse order.
     *
     * Rendering backend initialization and Vulkan resource ownership are
     * delegated to the renderer rather than managed directly by Application.
     */
    class Application {
      private:
        ApplicationConfiguration m_Configuration;
        Platform::GlfwContext m_GlfwContext;
        Platform::Window m_Window;
        Renderer::Vulkan::VulkanRenderer m_Renderer;

      public:
        /**
         * @brief Constructs the application and its engine-wide systems.
         *
         * Stores the supplied application configuration, initializes the
         * platform context, creates the primary application window, and
         * initializes the renderer for that window using the configured
         * presentation and diagnostic policies.
         *
         * @param configuration Top-level configuration controlling application
         * startup policy. The default value uses Signum's standard application
         * settings.
         *
         * @throws Error::EngineError if a required platform, window, or
         * rendering resource cannot be initialized.
         */
        explicit Application(ApplicationConfiguration configuration = {});

        /**
         * @brief Runs the application's primary event and rendering loop.
         *
         * Processes pending platform events, invokes the optional
         * application update callback, and renders each frame until
         * the primary window receives a close request.
         *
         * The update callback runs after event polling and before
         * rendering, allowing application state to change before
         * the current frame is recorded.
         *
         * @param on_update Optional callback invoked once per frame.
         * Receives a reference to the running application.
         *
         * @note An empty callback skips application-specific updates.
         */
        auto run(const std::function<void(Application &)> &on_update = {})
            -> void;

        /**
         * @brief Provides read-only access to the application's primary window.
         *
         * Allows callers to query window state and input through the
         * platform window interface without taking ownership.
         *
         * @return Reference to the application-owned primary window.
         *
         * @note The returned reference remains valid only while the
         * Application object is alive.
         */
        [[nodiscard]] auto window() const noexcept -> const Platform::Window &;

        /**
         * @brief Updates the model transformation of a renderer draw instance.
         *
         * Forwards the supplied model matrix to the application's renderer
         * without exposing rendering backend details to the caller.
         *
         * The updated transformation is used when recording subsequent
         * rendering commands.
         *
         * @param instance_index Zero-based index of the draw instance.
         * @param model_matrix Replacement model transformation.
         *
         * @pre instance_index must identify an existing draw instance.
         * @pre Updates must be serialized with rendering operations.
         *
         * @post The selected draw instance uses the supplied model matrix.
         *
         * @note This operation does not recreate or upload mesh resources.
         */
        auto setDrawInstanceModelMatrix(std::size_t instance_index,
                                        const Math::Matrix4f &model_matrix)
            -> void;

        /**
         * @brief Updates the camera view used for subsequent frames.
         *
         * Forwards a world-to-view transformation to the renderer.
         * Preserves the existing camera projection parameters.
         *
         * @param view_matrix Replacement world-to-view matrix.
         *
         * @pre A camera must already be configured.
         * @pre Every matrix element must be finite.
         * @pre Updates must be serialized with rendering operations.
         *
         * @post Subsequent frames use the supplied view matrix.
         *
         * @note Does not recreate renderer resources.
         */
        auto setCameraViewMatrix(const Math::Matrix4f &view_matrix) -> void;

        /**
         * @brief Updates an existing material uniform or storage buffer.
         *
         * Forwards the supplied descriptor location and CPU data to the
         * renderer, which creates a replacement GPU buffer and updates the
         * associated material descriptor resources.
         *
         * Previously submitted GPU work retains access to the old resources
         * until they are safe to retire.
         *
         * The operation does not recreate the graphics pipeline.
         *
         * @param input Descriptor location and borrowed replacement buffer
         * data.
         *
         * @pre The specified material binding must exist.
         * @pre The binding must reference a uniform or storage buffer.
         * @pre The supplied data must be nonempty and match the existing buffer
         * size.
         * @pre Updates must be serialized with rendering and resource
         * replacement.
         *
         * @post On success, subsequent frames use the updated buffer data.
         * @post On failure, the previously active material remains unchanged.
         *
         * @throws Error::EngineError if GPU resource creation fails.
         * @throws std::bad_alloc if resource allocation fails.
         *
         * @note The supplied CPU data must remain valid until this call
         * completes.
         */
        auto updateMaterialBuffer(Renderer::MaterialBufferUpdateInput input)
            -> void;

        /**
         * @brief Replaces the active material's fragment shader.
         *
         * Forwards the compiled shader to the renderer, which validates
         * its interface and creates a compatible graphics pipeline
         * before activating the replacement.
         *
         * @param fragment_shader Compiled shader bytecode and entry point.
         *
         * @pre An active material, mesh, and graphics pipeline must exist.
         * @pre The shader must select the fragment stage.
         * @pre The caller must serialize replacement with rendering and
         * other resource replacement operations.
         *
         * @post On success, subsequent frames use the replacement shader.
         *
         * @throws Error::EngineError if shader validation, compatibility
         * checks, or pipeline creation fails.
         * @throws std::bad_alloc if host resource allocation fails.
         */
        auto replaceFragmentShader(
            Renderer::Shader::ShaderStageInput fragment_shader) -> void;
    };
} // namespace SNE::Engine::Core
