#pragma once

#include "application_configuration.hpp"
#include "engine/core/logging/logger.hpp"
#include "engine/core/math/matrix4.hpp"
#include "engine/platform/event/platform_event.hpp"
#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/geometry/mesh_draw_instance.hpp"
#include "engine/renderer/geometry/mesh_upload_data.hpp"
#include "engine/renderer/geometry/vertex_layout_description.hpp"
#include "engine/renderer/material/material_buffer_update_input.hpp"
#include "engine/renderer/material/material_description.hpp"
#include "engine/renderer/render_target_extent.hpp"
#include "engine/renderer/resource/gpu_resource_handle.hpp"
#include "engine/renderer/shader/shader_stage_input.hpp"
#include "engine/renderer/vulkan/vulkan_external_rendering_info.hpp"
#include "engine/renderer/vulkan/vulkan_frame_command_recorder.hpp"
#include "engine/renderer/vulkan/vulkan_graphics_submission_progress.hpp"
#include "engine/renderer/vulkan/vulkan_renderer.hpp"
#include "engine/renderer/vulkan/vulkan_scene_viewport_image_info.hpp"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <vector>

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
        /**
         * @brief Owned configuration defining application initialization
         * policy.
         *
         * Stores the application name, initial window dimensions,
         * presentation preferences, and initial rendering resources.
         *
         * Constructed before other application subsystems so its
         * configuration remains available during their initialization.
         *
         * @note Configuration values may contain borrowed resource views
         * whose referenced storage must satisfy their documented lifetimes.
         */
        ApplicationConfiguration m_Configuration;

        /**
         * @brief Application-owned engine diagnostic logger.
         *
         * Owns the logging destinations configured through
         * ApplicationConfiguration and coordinates severity filtering,
         * category filtering, and diagnostic dispatch.
         *
         * Initialized after m_Configuration but before the platform
         * context, window, and renderer, allowing these subsystems
         * to emit diagnostics during initialization.
         *
         * The logger remains alive until all dependent application
         * subsystems have been destroyed.
         *
         * @note Logging destinations and severity are established
         * before platform or renderer initialization begins.
         */
        Logging::Logger m_Logger;

        /**
         * @brief Owns the process-wide GLFW initialization lifetime.
         *
         * Initializes the GLFW library before creating platform windows
         * and terminates it during application destruction.
         *
         * Declared before the window and renderer so GLFW remains
         * initialized throughout their lifetimes.
         *
         * @note GLFW initialization failures are propagated through
         * the platform's error-handling infrastructure.
         */
        Platform::GlfwContext m_GlfwContext;

        /**
         * @brief Owns the application's primary platform window.
         *
         * Manages the underlying GLFW window and provides access to
         * window state, input, and platform event handling.
         *
         * Created after GLFW initialization using the configured
         * application name and initial window dimensions.
         *
         * Declared before the renderer so the native window remains
         * valid throughout rendering resource initialization and
         * destruction.
         */
        Platform::Window m_Window;

        /**
         * @brief Owns the application's Vulkan rendering backend.
         *
         * Manages Vulkan rendering resources, frame execution,
         * presentation, and renderer-specific resource lifetimes.
         *
         * Initialized using the application configuration and
         * previously constructed platform window.
         *
         * Declared after its dependencies so Vulkan resources
         * are destroyed before the window and GLFW context.
         *
         * @note Vulkan resource management and synchronization
         * remain the responsibility of the rendering backend.
         */
        Renderer::Vulkan::VulkanRenderer m_Renderer;

      public:
        /**
         * @brief Constructs the application and its engine-wide systems.
         *
         * Stores the supplied application configuration, initializes
         * the configured logging destinations, creates the platform
         * context and window, and initializes the renderer.
         *
         * Logging destinations are initialized before GLFW and Vulkan
         * so diagnostic events emitted during startup can be captured.
         *
         * @param configuration Top-level configuration controlling
         * application initialization. The default value uses
         * Signum's standard application settings.
         *
         * @throws std::invalid_argument if the logging configuration
         * contains an invalid file rotation policy.
         * @throws Error::EngineError if a logging destination,
         * platform resource, window, or renderer resource cannot
         * be initialized.
         * @throws std::bad_alloc if host allocation fails.
         *
         * @note Logging configuration remains independent from
         * development diagnostics such as Vulkan validation.
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
         * @param record_external_commands Optional Vulkan command-recording
         * callback forwarded to the renderer during each drawable frame.
         * An empty callback preserves normal rendering behavior.
         * @param on_graphics_submission Optional callback invoked when
         * a new graphics submission is recorded and frame rendering
         * returns successfully.
         * @param on_platform_events Optional callback invoked after event
         * polling when pending platform events exist. Receives the application
         * and a read-only event span valid only for the duration of the
         * callback.
         *
         * @pre The external recording callback must not end the active
         * rendering scope, reset or submit the command buffer, or retain it
         * after invocation.
         * @pre The external recording callback must not throw exceptions.
         *
         * @note An empty callback skips application-specific updates.
         * @note A recorded submission is not necessarily complete.
         * Its GPU completion must be checked separately.
         */
        auto
        run(const std::function<void(Application &)> &on_update = {},
            const Renderer::Vulkan::VulkanFrameCommandRecorder
                &record_external_commands = {},
            const std::function<void(std::uint64_t)> &on_graphics_submission =
                {},
            const std::function<void(Application &,
                                     std::span<const Platform::PlatformEvent>)>
                &on_platform_events = {}) -> void;

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
         * @brief Requests the rendering extent of an optional scene viewport.
         *
         * Forwards the requested framebuffer-pixel dimensions to the renderer.
         *
         * An engaged extent enables offscreen scene rendering at the requested
         * dimensions. An empty extent indicates that no drawable scene viewport
         * is currently available.
         *
         * The renderer processes the request during frame rendering.
         * This function does not directly allocate GPU resources or record
         * rendering commands.
         *
         * @param extent Requested viewport dimensions in framebuffer pixels,
         * or std::nullopt when the viewport is not drawable.
         *
         * @pre An engaged extent must have nonzero width and height.
         * @pre Requests must be serialized with frame rendering.
         *
         * @post The renderer's requested scene viewport extent is updated.
         *
         * @note Existing offscreen GPU resources are managed separately
         * according to their outstanding usage and lifetime requirements.
         */
        auto setSceneViewportExtent(
            std::optional<Renderer::RenderTargetExtent> extent) -> void;

        /**
         * @brief Returns the current offscreen scene image information.
         *
         * Forwards the renderer's borrowed scene viewport image description
         * to external rendering consumers.
         *
         * Returns std::nullopt when no suitable offscreen scene image
         * is currently available.
         *
         * The returned information identifies the image view, sampling
         * layout, dimensions, frame slot, and resource generation.
         *
         * @return Borrowed scene viewport image information, or
         * std::nullopt when unavailable.
         *
         * @pre Access must be serialized with frame rendering and
         * renderer resource replacement.
         *
         * @note The application does not transfer ownership of
         * the underlying Vulkan image or image view.
         * @note Consumers must coordinate synchronization and resource
         * lifetimes before sampling the image.
         */
        [[nodiscard]] auto sceneViewportImageInfo() const
            -> std::optional<Renderer::Vulkan::VulkanSceneViewportImageInfo>;

        /**
         * @brief Returns the renderer's graphics submission progress.
         *
         * Forwards the latest submitted and confirmed completed graphics
         * submission identifiers from the application-owned renderer.
         *
         * Allows external rendering consumers to inspect graphics
         * completion progress without accessing the Vulkan renderer
         * directly.
         *
         * This operation reads existing submission bookkeeping and
         * does not wait for GPU execution.
         *
         * @return Snapshot of graphics submission and completion progress.
         *
         * @pre Access must be serialized with rendering operations.
         *
         * @note Completion information may lag behind actual GPU progress.
         * @note The returned snapshot does not guarantee completion of
         * submissions recorded after the snapshot was obtained.
         */
        [[nodiscard]] auto graphicsSubmissionProgress() const noexcept
            -> Renderer::Vulkan::VulkanGraphicsSubmissionProgress;

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

        /**
         * @brief Captures or releases the application window's cursor.
         *
         * Forwards the requested cursor mode to the application-owned window.
         *
         * @param captured true to capture the cursor; false to release it.
         */
        auto setCursorCaptured(bool captured) -> void;

        /**
         * @brief Returns Vulkan rendering information for external systems.
         *
         * Forwards the request to the application-owned Vulkan renderer.
         *
         * The returned information contains borrowed Vulkan handles and a
         * snapshot of the current swapchain configuration.
         *
         * @return Vulkan initialization and swapchain information.
         *
         * @pre The renderer's swapchain resources must be initialized.
         *
         * @note The returned handles remain owned by the renderer.
         * Swapchain-dependent information may become outdated following
         * swapchain recreation.
         */
        [[nodiscard]] auto externalRenderingInfo() const
            -> Renderer::Vulkan::VulkanExternalRenderingInfo;

        /**
         * @brief Requests that the application exit its main loop.
         *
         * Forwards the close request to the application-owned window.
         *
         * Does not immediately destroy application resources.
         * Normal resource cleanup occurs after the application loop exits.
         */
        auto requestClose() -> void;

        /**
         * @brief Registers mesh geometry with the application's renderer.
         *
         * Uploads the supplied mesh using its explicit vertex layout
         * and returns a handle identifying the registered GPU resource.
         *
         * Mesh storage and ownership remain managed by the renderer.
         *
         * @param mesh Borrowed mesh vertex and index data to upload.
         * @param layout Vertex attribute layout describing the mesh.
         *
         * @return Handle identifying the registered GPU mesh.
         *
         * @pre The mesh and vertex layout must satisfy the renderer's
         * mesh registration requirements.
         * @pre Resource registration must be serialized with rendering.
         *
         * @post On success, the mesh is available for renderer draw instances.
         *
         * @throws Error::EngineError if GPU resource allocation or
         * upload fails.
         * @throws std::bad_alloc if host allocation fails.
         *
         * @note The supplied CPU data must remain valid until
         * registration completes.
         */
        [[nodiscard]] auto
        registerMesh(const Renderer::MeshUploadData &mesh,
                     const Renderer::VertexLayoutDescription &layout)
            -> Renderer::GpuResourceHandle;

        /**
         * @brief Registers a graphics material configuration with the renderer.
         *
         * Stores the supplied shader selections and graphics pipeline state
         * in the renderer's material configuration registry.
         *
         * The returned handle can be assigned to draw instances to select
         * a graphics pipeline independently of the default material.
         *
         * Graphics pipeline creation is deferred until the configuration
         * is required for rendering.
         *
         * @param description Material configuration to register.
         *
         * @return Handle identifying the registered material configuration.
         *
         * @pre Shader stage inputs must satisfy the renderer's validation
         * requirements.
         * @pre Registration must be serialized with rendering and other
         * resource operations.
         *
         * @post On success, the configuration is available for draw instances.
         * @post The renderer owns the registered configuration.
         *
         * @throws Error::EngineError if shader validation fails.
         * @throws std::bad_alloc if registry allocation fails.
         *
         * @note Registration does not immediately create a graphics pipeline.
         */
        [[nodiscard]] auto
        registerMaterialConfiguration(Renderer::MaterialDescription description)
            -> Renderer::GpuResourceHandle;

        /**
         * @brief Replaces the application's renderer draw instances.
         *
         * Forwards a collection of mesh draw descriptions to the renderer,
         * which validates their resource handles and stores an independent
         * copy for subsequent frames.
         *
         * Each instance selects a registered mesh, model transformation,
         * optional material instance, and optional material configuration.
         *
         * Passing an empty collection removes all draw instances.
         *
         * @param instances Borrowed draw descriptions to install.
         *
         * @pre Every mesh handle must identify a registered mesh.
         * @pre Every supplied material instance handle must identify a
         * registered material instance.
         * @pre Every supplied material configuration handle must identify
         * a registered material configuration.
         * @pre Updates must be serialized with rendering and resource
         * operations.
         *
         * @post On success, the renderer uses exactly the supplied draw
         * instances.
         * @post On allocation failure, the previous draw instances remain
         * unchanged.
         *
         * @throws std::bad_alloc if copying the draw instances fails.
         *
         * @note Mesh and material resources remain owned by the renderer.
         * @note This operation does not upload mesh data or create pipelines.
         */
        auto
        setDrawInstances(std::span<const Renderer::MeshDrawInstance> instances)
            -> void;

        /**
         * @brief Returns a snapshot of the application's renderer draw
         * instances.
         *
         * Forwards the request to the renderer and returns an independent
         * copy of its currently configured mesh draw instances.
         *
         * Each returned instance preserves its mesh handle, model
         * transformation, optional material instance, and optional
         * material configuration.
         *
         * Modifying the returned collection does not affect renderer state.
         *
         * @return Owned copy of the current draw instances, preserving
         * their rendering order.
         *
         * @pre Access must be serialized with rendering and draw-instance
         * modifications.
         *
         * @post The renderer's draw instances remain unchanged.
         *
         * @throws std::bad_alloc if snapshot allocation fails.
         *
         * @note Returned resource handles are non-owning references
         * to renderer-managed resources.
         * @note The snapshot may become outdated after subsequent
         * renderer modifications.
         * @note This operation does not allocate or modify GPU resources.
         */
        [[nodiscard]] auto drawInstances() const
            -> std::vector<Renderer::MeshDrawInstance>;
    };
} // namespace SNE::Engine::Core
