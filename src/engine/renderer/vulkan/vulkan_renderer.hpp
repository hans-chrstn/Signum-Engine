#pragma once

#include "engine/core/math/matrix4.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/camera/initial_camera_data.hpp"
#include "engine/renderer/geometry/initial_mesh_data.hpp"
#include "engine/renderer/geometry/mesh_upload_data.hpp"
#include "engine/renderer/geometry/vertex_attribute_description.hpp"
#include "engine/renderer/material/material_description.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/resource/gpu_resource_registry.hpp"
#include "engine/renderer/shader/graphics_shader_data.hpp"
#include "engine/renderer/texture/texture_upload_data.hpp"
#include "engine/renderer/vulkan/command/vulkan_immediate_submission.hpp"
#include "engine/renderer/vulkan/descriptor/vulkan_descriptor_pool.hpp"
#include "engine/renderer/vulkan/descriptor/vulkan_descriptor_set_layout.hpp"
#include "engine/renderer/vulkan/device/vulkan_device.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_features.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_selection.hpp"
#include "engine/renderer/vulkan/memory/vulkan_buffer.hpp"
#include "engine/renderer/vulkan/memory/vulkan_image.hpp"
#include "engine/renderer/vulkan/memory/vulkan_image_view.hpp"
#include "engine/renderer/vulkan/memory/vulkan_memory_allocator.hpp"
#include "engine/renderer/vulkan/memory/vulkan_sampler.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_graphics_pipeline.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_graphics_pipeline_cache.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_surface.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_swapchain.hpp"
#include "engine/renderer/vulkan/synchronization/vulkan_queue_submission_state.hpp"
#include "engine/renderer/vulkan/synchronization/vulkan_resource_retirement_queue.hpp"
#include "engine/renderer/vulkan/synchronization/vulkan_semaphore.hpp"
#include "vulkan_frame_resources.hpp"
#include "vulkan_instance.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Owns and coordinates the Vulkan renderer runtime.
     *
     * Establishes the lifetime of the Vulkan instance, presentation surface,
     * selected physical-device state, logical device, GPU memory allocator,
     * swapchain, graphics pipeline, and command infrastructure required by the
     * renderer.
     *
     * Renderer resources are stored in dependency order so they are constructed
     * from lower-level Vulkan dependencies to higher-level resources and
     * destroyed safely in reverse order.
     *
     * The application window is borrowed during renderer initialization and
     * must outlive the renderer.
     *
     * Renderer queue operations are currently performed serially by the
     * renderer-owning thread. Renderer queues must not be accessed concurrently
     * by other host threads.
     *
     * The type is non-copyable because it owns Vulkan resources with exclusive
     * lifetimes.
     */
    class VulkanRenderer {
      private:
        /**
         * @brief Owns the Vulkan resources for a sampled texture.
         *
         * Groups the GPU image, its image view, and the sampler used
         * to access the texture from shaders.
         *
         * The image view references the owned image, while the sampler
         * defines filtering and texture-coordinate addressing behavior.
         *
         * Members are declared in dependency order so the image view
         * is destroyed before the underlying image.
         *
         * The structure is move-only because its Vulkan resources
         * have exclusive ownership.
         */
        struct TextureResources {
            /** @brief Owns the GPU image containing the texture pixels. */
            VulkanImage m_Image;

            /** @brief Owns the image view used to access the texture. */
            VulkanImageView m_ImageView;

            /** @brief Owns the sampler used for texture sampling. */
            VulkanSampler m_Sampler;
        };

        static_assert(std::is_nothrow_move_constructible_v<TextureResources>);

        /**
         * @brief Owns a sampled texture and its descriptor binding
         * resources.
         *
         * Groups the texture image, image view, sampler, and descriptor
         * pool required to bind the texture during rendering.
         *
         * The descriptor set is a non-owning handle allocated from the
         * descriptor pool. Destroying the pool implicitly frees the set.
         *
         * Members are declared in dependency order so the descriptor pool
         * is destroyed before the texture resources it references.
         *
         * The structure is move-only because its Vulkan resources have
         * exclusive ownership.
         */
        struct TextureBindingResources {
            /** @brief Owns the texture image, image view, and sampler. */
            TextureResources m_Resources;

            /** @brief Owns the descriptor pool and its allocated descriptor
             * sets. */
            VulkanDescriptorPool m_DescriptorPool;

            /**
             * @brief Non-owning descriptor-set handle allocated from
             * m_DescriptorPool.
             *
             * Remains valid while the descriptor pool exists and is not reset.
             */
            VkDescriptorSet m_DescriptorSet{VK_NULL_HANDLE};

            /**
             * @brief Identifies the registered texture referenced by this
             * binding.
             *
             * References a texture owned by m_TextureRegistry without taking
             * ownership of its GPU resources.
             *
             * The handle must resolve to a registered texture for the binding
             * to remain valid.
             */
            GpuResourceHandle m_TextureHandle{};
        };

        static_assert(
            std::is_nothrow_move_constructible_v<TextureBindingResources>);

        /**
         * @brief Owns a depth image and its corresponding image view.
         *
         * Groups the GPU image and image-view resources required for
         * a depth attachment.
         *
         * The image view references the image owned by this structure.
         * Members are declared in dependency order so the image view
         * is destroyed before the underlying image.
         *
         * The structure is move-only because its resources have
         * exclusive Vulkan ownership.
         */
        struct DepthAttachmentResources {
            /** @brief Owns the VMA-backed depth image. */
            VulkanImage m_Image;

            /** @brief Owns the image view referencing the depth image. */
            VulkanImageView m_ImageView;
        };

        /**
         * @brief Owns the GPU buffers and index count of a mesh.
         *
         * Groups the vertex buffer, index buffer, and associated index count
         * into one resource ownership unit.
         *
         * Both buffers are owned exclusively and must remain alive while GPU
         * submissions reference them.
         *
         * The structure supports moving its resources together during mesh
         * replacement and deferred GPU resource retirement.
         *
         * The structure is move-only because its Vulkan buffers have
         * exclusive ownership.
         */
        struct MeshBufferResources {
            /** @brief Owns the GPU vertex buffer. */
            VulkanBuffer m_VertexBuffer;

            /** @brief Owns the GPU index buffer. */
            VulkanBuffer m_IndexBuffer;

            /** @brief Number of indices stored in the index buffer. */
            std::uint32_t m_IndexCount{};
        };

        static_assert(
            std::is_nothrow_move_constructible_v<MeshBufferResources>);

        /**
         * @brief Owns resources associated with a presentation swapchain.
         *
         * Groups the Vulkan swapchain, a borrowed graphics pipeline,
         * presentation synchronization objects, and depth attachments.
         *
         * The graphics pipeline is owned by VulkanGraphicsPipelineCache
         * and remains valid across swapchain recreation.
         *
         * Render-finished semaphores are maintained per swapchain image,
         * while depth attachments are maintained per frame-in-flight slot.
         *
         * Resources must not be destroyed while outstanding GPU
         * operations still reference them.
         */
        struct SwapchainResources {
            /**
             * @brief Owns the Vulkan presentation swapchain.
             *
             * Manages the swapchain, its presentation images, and their
             * associated image views.
             *
             * Determines the image count, color format, and extent used
             * by the other swapchain-dependent resources.
             */
            VulkanSwapchain m_VulkanSwapchain;

            /**
             * @brief Depth attachment format associated with this swapchain.
             *
             * Records the format used by the owned depth attachments and
             * graphics pipeline to determine whether pipeline recreation
             * is required when attachment formats change.
             */
            VkFormat m_DepthFormat{VK_FORMAT_UNDEFINED};

            /**
             * @brief Non-owning pointer to the graphics pipeline used for
             * rendering.
             *
             * References a pipeline owned by m_GraphicsPipelineCache.
             *
             * Remains null when no graphics pipeline is required.
             *
             * The cache must outlive this pointer and must not remove its
             * referenced pipeline while it is in use by the GPU.
             */
            const VulkanGraphicsPipeline *m_VulkanGraphicsPipeline{nullptr};

            /**
             * @brief Owns render-finished semaphores for swapchain images.
             *
             * Maintains one semaphore per swapchain image to synchronize
             * completed rendering with presentation.
             *
             * Each semaphore is associated with its corresponding
             * swapchain image index.
             *
             * The collection is recreated when swapchain-dependent
             * resources are rebuilt.
             */
            std::vector<VulkanSemaphore> m_VulkanSemaphores;

            /**
             * @brief Owns depth attachments for frame-in-flight slots.
             *
             * Each frame slot receives an independent depth image and
             * image view, preventing overlapping frames from writing
             * to the same depth attachment.
             *
             * Every depth image uses a supported depth format and
             * matches the current swapchain extent.
             *
             * The collection is recreated alongside the swapchain.
             */
            std::vector<DepthAttachmentResources> m_DepthAttachments;
        };

        static_assert(std::is_nothrow_move_constructible_v<SwapchainResources>);

        /**
         * @brief Application window borrowed by the renderer.
         *
         * Provides access to the current framebuffer dimensions required for
         * presentation and swapchain recreation.
         *
         * The window is not owned by the renderer and must remain valid for the
         * entire lifetime of this VulkanRenderer.
         */
        const Platform::Window *m_Window{nullptr};
        /**
         * @brief Vulkan instance owned by the renderer.
         *
         * Establishes the Vulkan runtime context required by all renderer-owned
         * Vulkan resources and outlives every resource created from it.
         */
        VulkanInstance m_Instance;
        /**
         * @brief Presentation surface owned by the renderer.
         *
         * Connects the Vulkan instance to the application window and remains
         * valid for the lifetime of presentation-dependent renderer resources.
         */
        VulkanSurface m_Surface;
        /**
         * @brief Selected physical-device state used by the renderer.
         *
         * Stores the Vulkan physical-device handle together with the
         * capabilities and queue-family guarantees established during device
         * selection.
         *
         * The physical-device handle is owned by the Vulkan instance rather
         * than by this object.
         */
        SelectedPhysicalDevice m_PhysicalDevice;

        /**
         * @brief Presentation behavior requested from the renderer.
         *
         * Stores the backend-independent presentation preference used when
         * creating the initial swapchain and when recreating
         * swapchain-dependent resources.
         *
         * The preference is retained for the lifetime of the renderer so
         * presentation mode selection remains consistent across swapchain
         * recreation.
         */
        PresentationPreference m_PresentationPreference;

        /**
         * @brief Vulkan features selected for logical-device creation.
         *
         * Stores the feature configuration derived from the selected
         * physical-device capabilities and renderer policy.
         *
         * The configuration is retained for renderer behavior that depends on
         * which optional Vulkan features were actually enabled.
         */
        LogicalDeviceFeatureConfiguration m_LogicalDeviceFeatureConfiguration;
        /**
         * @brief Logical Vulkan device owned by the renderer.
         *
         * Owns the VkDevice and provides the graphics and presentation queues
         * used by renderer operations.
         *
         * The device outlives all renderer resources created from it.
         */
        VulkanDevice m_Device;
        /**
         * @brief Immediate submission resources owned by the renderer.
         *
         * Owns a command pool, command buffer, and fence used to execute
         * synchronous one-time Vulkan work on the graphics queue.
         *
         * The graphics queue is borrowed from the logical device. The logical
         * device and queue must remain valid for the lifetime of this object.
         */
        VulkanImmediateSubmission m_ImmediateSubmission;
        /**
         * @brief Vulkan memory allocator owned by the renderer.
         *
         * Owns the VMA allocator used to manage memory backing renderer-created
         * Vulkan buffers and images.
         *
         * The Vulkan instance, selected physical device, and logical device are
         * borrowed by the allocator and must remain valid for its lifetime.
         *
         * Resources and allocations created through this allocator must be
         * released before the allocator is destroyed.
         */
        VulkanMemoryAllocator m_MemoryAllocator;

        /**
         * @brief Owns the descriptor-set layout for the initial sampled
         * texture.
         *
         * Describes the resource binding expected by the textured graphics
         * pipeline.
         *
         * The layout remains valid across swapchain recreation and must
         * outlive graphics pipeline layouts created from it.
         *
         * Empty when no texture descriptor layout has been created.
         */
        std::optional<VulkanDescriptorSetLayout> m_TextureDescriptorSetLayout;

        /**
         * @brief Owns reusable Vulkan graphics pipelines.
         *
         * Stores pipelines identified by their complete variant configurations.
         *
         * The cache outlives swapchain resources that borrow its pipelines
         * and is destroyed before the logical device.
         *
         * Cached pipelines remain available across swapchain recreation.
         */
        VulkanGraphicsPipelineCache m_GraphicsPipelineCache;

        /**
         * @brief Owns the renderer's registered GPU texture resources.
         *
         * Stores texture images, image views, and samplers using
         * generation-checked, registry-scoped handles.
         *
         * Descriptor bindings may reference registered textures without
         * taking ownership of their GPU resources.
         *
         * Registered textures must remain alive while descriptor bindings
         * or outstanding GPU submissions reference them.
         *
         * Resources still referenced by GPU submissions must be retired
         * before their registry ownership is released.
         */
        GpuResourceRegistry<TextureResources> m_TextureRegistry;

        /**
         * @brief Owns the renderer's active sampled texture and descriptor
         * binding.
         *
         * Groups the texture resources, descriptor pool, and allocated
         * descriptor set into one ownership unit.
         *
         * The resources remain valid across swapchain recreation and must not
         * be destroyed while outstanding GPU submissions reference them.
         *
         * Replaced resources are retained until their associated GPU work
         * completes.
         *
         * Empty when no texture has been initialized.
         */
        std::optional<TextureBindingResources> m_TextureBindingResources;

        /**
         * @brief Identifies the texture currently selected for rendering.
         *
         * References a resource owned by m_TextureRegistry without taking
         * ownership.
         *
         * Empty when no active texture has been selected.
         * The handle must resolve to a valid registered texture before use.
         */
        std::optional<GpuResourceHandle> m_ActiveTextureHandle;

        /**
         * @brief Owns the renderer's registered GPU mesh resources.
         *
         * Stores mesh buffers and index counts using generation-checked,
         * registry-scoped handles.
         *
         * Registered meshes remain owned until explicitly removed or until
         * the registry is destroyed.
         *
         * GPU submissions must complete before referenced resources are
         * destroyed.
         */
        GpuResourceRegistry<MeshBufferResources> m_MeshRegistry;

        /**
         * @brief Identifies the mesh currently selected for rendering.
         *
         * References a resource owned by m_MeshRegistry without taking
         * ownership.
         *
         * Empty when no active mesh has been selected.
         * The handle must resolve to a valid resource before drawing.
         */
        std::optional<GpuResourceHandle> m_ActiveMeshHandle;

        /**
         * @brief Owns the model transformations for the uploaded mesh.
         *
         * Stores copies of application-provided model matrices so rendering
         * does not depend on caller-managed CPU storage.
         *
         * Each matrix represents one draw of the same uploaded mesh.
         *
         * Empty when no mesh instances have been configured.
         */
        std::vector<Core::Math::Matrix4f> m_ModelMatrices;

        /**
         * @brief Owns the vertex attribute descriptions of the uploaded mesh.
         *
         * Stores a copy of the caller-provided vertex layout so graphics
         * pipelines can be recreated without referencing expired external
         * storage.
         *
         * Empty when no mesh layout has been configured.
         */
        std::vector<VertexAttributeDescription> m_MeshVertexAttributes;

        /**
         * @brief Byte stride of each vertex in the uploaded mesh.
         *
         * Retained alongside the owned attribute descriptions for
         * graphics-pipeline creation and recreation.
         *
         * Zero when no mesh has been configured.
         */
        std::uint32_t m_MeshVertexStride{};

        /**
         * @brief Optional initial camera configuration owned by the renderer.
         *
         * Stores a copy of the application-provided view transformation and
         * perspective projection parameters.
         *
         * The renderer derives the projection aspect ratio from the current
         * drawable framebuffer dimensions.
         *
         * The camera configuration is independent of swapchain resources and
         * remains available across swapchain recreation.
         *
         * Empty when no initial camera has been configured.
         */
        std::optional<InitialCameraData> m_InitialCamera;

        /**
         * @brief Optional material configuration owned by the renderer.
         *
         * Retains the material's shader bytecode, entry-point selections,
         * and graphics pipeline state independently of Vulkan resources.
         *
         * The configuration remains available for pipeline creation
         * and swapchain recreation.
         *
         * Empty when no application-provided material is configured.
         */
        std::optional<MaterialDescription> m_Material;

        /**
         * @brief Optional swapchain-dependent renderer resources.
         *
         * Owns the presentation swapchain, synchronization objects,
         * and depth attachments while borrowing its graphics pipeline
         * from m_GraphicsPipelineCache.
         *
         * Empty when no drawable framebuffer is available.
         *
         * The graphics pipeline pointer remains null when no
         * renderable geometry is configured.
         */
        std::optional<SwapchainResources> m_SwapchainResources;
        /**
         * @brief Reusable resource sets for frames that may be in flight
         * concurrently.
         *
         * Stores one VulkanFrameResources object for each active
         * frame-in-flight slot. Each slot independently owns its command
         * resources and frame-scoped synchronization objects.
         *
         * The collection size represents renderer frame-reuse policy and is
         * independent from the number of images owned by the swapchain.
         */
        std::vector<VulkanFrameResources> m_FrameResources;

        /**
         * @brief Tracks graphics queue submission and completion state.
         *
         * Associates graphics submissions with reusable frame-in-flight slots
         * and records completion confirmed by their corresponding fences.
         *
         * Maintains CPU-side submission identifiers without owning Vulkan
         * queues, fences, command buffers, or GPU resources.
         *
         * The tracked frame-slot count matches m_FrameResources.
         *
         * This state is specific to the graphics queue submission stream.
         * Independent compute or transfer queues require separate completion
         * tracking unless their synchronization dependencies are established.
         *
         * The renderer is responsible for validating submissions before
         * queue submission and recording completion only after GPU execution
         * has been confirmed.
         */
        VulkanQueueSubmissionState m_GraphicsSubmissionState;

        /**
         * @brief Retains retired graphics resources until GPU work completes.
         *
         * Owns deferred resource-release operations associated with graphics
         * submission identifiers tracked by m_GraphicsSubmissionState.
         *
         * Resources remain retained while their associated submissions are
         * incomplete and are released after confirmed GPU completion.
         *
         * This queue owns retired resources but does not own Vulkan queues,
         * fences, command buffers, or synchronization objects.
         *
         * The renderer is responsible for collecting completed resources during
         * frame processing, swapchain recreation, and shutdown.
         *
         * All pending retirement records must be collected before this
         * member is destroyed.
         */
        VulkanResourceRetirementQueue m_GraphicsResourceRetirementQueue;

        /**
         * @brief Index of the frame-in-flight slot currently being processed.
         *
         * Selects the VulkanFrameResources object used for the current renderer
         * frame. The index advances after a frame is completed and wraps
         * through the available frame-resource slots.
         */
        std::size_t m_CurrentFrameIndex{};

        /**
         * @brief Indicates that a queue submission failed and rendering
         * must not continue using this renderer instance.
         *
         * Prevents subsequent frames from waiting on synchronization
         * objects whose expected GPU signal may never occur.
         */
        bool m_SubmissionFailed{false};

        /**
         * @brief Waits until the application framebuffer becomes drawable.
         *
         * Blocks on platform window events while the framebuffer has a
         * non-positive width or height.
         *
         * Waiting stops when the framebuffer becomes drawable or when the
         * application window is requested to close.
         *
         * @return true if the framebuffer is drawable when waiting ends;
         * otherwise false if the window is closing.
         */
        [[nodiscard]] auto waitForDrawableFramebuffer() const -> bool;

        /**
         * @brief Reports whether the application framebuffer can currently be
         * rendered to.
         *
         * Queries the borrowed application window's current framebuffer
         * dimensions. A framebuffer is drawable only when both its width and
         * height are greater than zero.
         *
         * @return true when both framebuffer dimensions are positive; otherwise
         * false.
         */
        [[nodiscard]] auto isFramebufferDrawable() const -> bool;

        /**
         * @brief Recreates resources that depend on the presentation swapchain.
         *
         * Waits until the application framebuffer is drawable, synchronizes
         * with outstanding device work, and rebuilds swapchain-dependent
         * renderer resources using the current framebuffer dimensions and
         * presentation configuration.
         *
         * Frame-in-flight resources are preserved because their lifetime is
         * independent of swapchain image count.
         *
         * @return true if swapchain-dependent resources were successfully
         * recreated; otherwise false if recreation was abandoned because the
         * window is closing.
         *
         * @throws Core::Error::EngineError if required Vulkan synchronization
         * or resource recreation fails.
         * @post Successful recreation installs a new swapchain with compatible
         * depth attachments, presentation synchronization, and graphics
         * pipeline resources.
         *
         * @note If resource recreation throws, the previously owned
         * swapchain resources are released because the old swapchain may
         * already have been retired. The original exception propagates
         * to the caller.
         */
        [[nodiscard]] auto recreateSwapchainResources() -> bool;

        /**
         * @brief Creates, uploads, and registers GPU mesh buffers.
         *
         * Validates the supplied mesh data and uploads vertex and index buffers
         * using the renderer's memory allocator and immediate submission
         * resources.
         *
         * On initial upload, registers the mesh and selects its logical handle.
         * On replacement, registers the new mesh before retiring the previous
         * GPU resources. Retired resources remain alive until the graphics
         * submissions referencing them have completed.
         *
         * The supplied CPU storage is borrowed only for the duration of the
         * upload operation.
         *
         * @param mesh Non-owning vertex data, vertex stride, and index data.
         *
         * @pre Vertex data and indices must be nonempty.
         * @pre The index count must be representable by std::uint32_t.
         * @pre Vertex stride must be nonzero and divide the vertex byte count
         * evenly.
         * @pre Every index must reference an existing vertex.
         * @pre The memory allocator and immediate submission resources must
         * be initialized.
         * @pre Upload and replacement operations must be serialized with
         * frame recording and graphics queue submission.
         *
         * @post On success, the active handle identifies the newly registered
         * mesh containing the uploaded GPU buffers.
         * @post If replacement preparation or retirement fails, the previous
         * mesh remains registered and active.
         *
         * @throws Core::Error::EngineError if Vulkan resource creation,
         * allocation, or upload fails.
         * @throws std::bad_alloc if registry insertion or retirement
         * preparation fails due to memory allocation.
         *
         * @note Invalid mesh descriptions trigger fatal precondition
         * assertions.
         */
        auto uploadMesh(const MeshUploadData &mesh) -> void;

        /**
         * @brief Records Vulkan rendering commands for an acquired swapchain
         * image.
         *
         * Records attachment layout transitions, dynamic rendering, mesh draws,
         * and the final transition to presentation layout.
         *
         * @param command_buffer Command buffer to record into.
         * @param swapchain_resources Borrowed swapchain-dependent resources.
         * @param depth_attachment Depth attachment owned by the current frame
         * slot.
         * @param image_index Index of the acquired swapchain image.
         *
         * @pre command_buffer must be valid and ready for recording.
         * @pre image_index must reference an image in swapchain_resources.
         *
         * @throws Core::Error::EngineError if command buffer recording
         * initialization or completion fails.
         */
        auto
        recordFrameCommands(VkCommandBuffer command_buffer,
                            const SwapchainResources &swapchain_resources,
                            const DepthAttachmentResources &depth_attachment,
                            std::size_t image_index) -> void;

        /**
         * @brief Records indexed mesh drawing commands within active dynamic
         * rendering.
         *
         * Validates the required graphics resources, binds the graphics
         * pipeline and mesh buffers, configures the viewport and scissor,
         * calculates camera and model transformations, and records indexed
         * drawing commands.
         *
         * The function borrows existing renderer-owned resources and does not
         * create, destroy, or transfer ownership of Vulkan resources.
         *
         * @param command_buffer Command buffer currently recording graphics
         * commands.
         * @param swapchain_resources Swapchain resources providing the graphics
         * pipeline and current rendering extent.
         *
         * @pre command_buffer must be valid and recording within an active
         * dynamic-rendering instance.
         * @pre The renderer must have an uploaded mesh available for drawing.
         *
         * @note Missing required graphics resources trigger invariant
         * assertions.
         */
        auto
        recordMeshDraws(VkCommandBuffer command_buffer,
                        const SwapchainResources &swapchain_resources) const
            -> void;

        /**
         * @brief Submits a recorded frame command buffer to the graphics queue.
         *
         * Configures image-availability and render-completion semaphore
         * synchronization, resets the frame's in-flight fence, and submits
         * the recorded commands using Vulkan synchronization2.
         *
         * A failed queue submission permanently prevents further rendering
         * through this renderer instance.
         *
         * @param current_frame Frame resources owning the synchronization fence
         * and image-availability semaphore.
         * @param command_buffer Recorded command buffer to submit.
         * @param render_finished_semaphore Semaphore signaled after submitted
         * rendering commands complete.
         *
         * @pre command_buffer must be valid and executable.
         * @pre The frame's in-flight fence must be signaled before submission.
         * @pre render_finished_semaphore must be a valid Vulkan semaphore.
         *
         * @throws Core::Error::EngineError if fence reset or queue
         * submission fails.
         *
         * @note Queue submission failures are not automatically recovered.
         */
        auto submitFrame(VulkanFrameResources &current_frame,
                         VkCommandBuffer command_buffer,
                         VkSemaphore render_finished_semaphore) -> void;

        /**
         * @brief Creates and uploads a texture to GPU memory.
         *
         * Validates the supplied RGBA8 texture description, creates
         * a device-local Vulkan image, uploads the pixel data, and
         * initializes the corresponding image view and sampler.
         *
         * The uploaded image is transitioned to a layout suitable
         * for fragment-shader sampling.
         *
         * @param texture Borrowed texture dimensions and RGBA8 pixel data.
         *
         * @pre Texture width and height must be nonzero.
         * @pre Pixel storage must contain exactly width * height * 4 bytes.
         * @pre The renderer's allocator and immediate submission
         * infrastructure must be initialized.
         *
         * @post On success, the returned texture contains valid GPU resources.
         *
         * @return Owned texture image, image view, and sampler.
         *
         * @throws Core::Error::EngineError if Vulkan resource creation,
         * allocation, or upload fails.
         *
         * @note Invalid texture descriptions trigger precondition assertions.
         */
        auto uploadTexture(const TextureUploadData &texture)
            -> TextureResources;

        [[nodiscard]] auto
        createTextureBindingResources(TextureResources &&texture,
                                      const VulkanDescriptorSetLayout &layout)
            -> TextureBindingResources;

      public:
        /**
         * @brief Initializes the Vulkan renderer for an application window.
         *
         * Creates the Vulkan instance and presentation surface, selects a
         * suitable physical device, negotiates logical-device features, creates
         * the logical device and GPU memory allocator, and establishes the
         * initial command infrastructure.
         *
         * Swapchain-dependent resources are created during initialization when
         * the application framebuffer is drawable. If the framebuffer is not
         * drawable, their creation is deferred until rendering can begin.
         *
         * @param application_name Name reported to Vulkan for the application.
         * @param window Platform window used for Vulkan surface creation and
         * framebuffer sizing.
         * @param presentation_preference Presentation behavior requested by the
         * renderer.
         * @param development_diagnostics_enabled Whether development-oriented
         * Vulkan diagnostics should be enabled.
         * @param initial_mesh Optional borrowed mesh and vertex layout consumed
         * during renderer initialization. When absent, the renderer starts
         * without application-provided geometry. The underlying CPU storage
         * must remain valid until construction completes.
         * @param initial_texture Optional borrowed RGBA8 texture data consumed
         * during renderer initialization. The referenced pixel storage must
         * remain valid until construction completes.
         * @param initial_camera Optional initial camera configuration
         * containing the view transformation and perspective projection
         * parameters. The configuration is retained by value for subsequent
         * rendering.
         * @param initial_graphics_shaders Optional vertex and fragment shaders
         * supplied by the application. Ownership of the compiled SPIR-V
         * artifacts is transferred to the renderer for pipeline creation
         * and recreation.
         *
         * @throws Core::Error::EngineError if a required Vulkan resource or
         * capability cannot be initialized.
         */
        VulkanRenderer(
            const std::string &application_name, const Platform::Window &window,
            PresentationPreference presentation_preference,
            bool development_diagnostics_enabled,
            std::optional<InitialMeshData> initial_mesh = std::nullopt,
            std::optional<TextureUploadData> initial_texture = std::nullopt,
            std::optional<InitialCameraData> initial_camera = std::nullopt,
            std::optional<Shader::GraphicsShaderData> initial_graphics_shaders =
                std::nullopt);

        /**
         * @brief Releases the Vulkan renderer runtime.
         *
         * Waits for outstanding Vulkan device work to complete before
         * renderer-owned frame, synchronization, graphics, swapchain,
         * memory-allocation, and device resources are destroyed in reverse
         * dependency order.
         *
         * @note Failure to confirm GPU completion during shutdown triggers
         * a fatal invariant assertion. Outstanding retired resources must
         * be collected before renderer members are destroyed.
         */
        ~VulkanRenderer() noexcept;

        VulkanRenderer(const VulkanRenderer &) = delete;

        auto operator=(const VulkanRenderer &) -> VulkanRenderer & = delete;

        /**
         * @brief Replaces the currently active GPU mesh.
         *
         * Creates and uploads replacement vertex and index buffers before
         * changing the active mesh.
         *
         * The previous mesh is retained until graphics submissions that
         * reference it have completed.
         *
         * The replacement must use the existing vertex layout and stride.
         * This operation does not recreate the graphics pipeline.
         *
         * The operation must not overlap frame recording, graphics queue
         * submission, or another mesh replacement.
         *
         * @param mesh Borrowed vertex and index data for the replacement mesh.
         *
         * @pre An active mesh and its vertex layout must already exist.
         * @pre The replacement vertex stride must match the active layout.
         * @pre The caller must serialize this operation with rendering.
         *
         * @post On success, subsequent frames use the replacement mesh.
         * @post If preparation or retirement fails, the previous mesh
         * remains active.
         *
         * @throws Core::Error::EngineError if buffer creation or upload fails.
         *
         * @note The supplied CPU data must remain valid until this
         * operation completes.
         */
        auto replaceMesh(const MeshUploadData &mesh) -> void;

        /**
         * @brief Replaces the currently active sampled texture.
         *
         * Creates and uploads a replacement texture and descriptor binding
         * before changing the active resources.
         *
         * The previous texture and descriptor resources remain alive until
         * graphics submissions referencing them have completed.
         *
         * The replacement uses the existing descriptor-set layout and does
         * not recreate the graphics pipeline.
         *
         * @param texture Borrowed RGBA8 texture data for the replacement.
         *
         * @pre An active texture binding and descriptor-set layout must exist.
         * @pre The caller must serialize replacement with rendering and other
         * resource replacement operations.
         *
         * @post On success, subsequent frames use the replacement texture.
         * @post If preparation or retirement fails, the previous texture
         * remains active.
         *
         * @throws Core::Error::EngineError if Vulkan resource creation,
         * allocation, or upload fails.
         *
         * @note The supplied CPU data must remain valid until this
         * operation completes.
         */
        auto replaceTexture(const TextureUploadData &texture) -> void;

        /**
         * @brief Processes one renderer frame.
         *
         * Coordinates synchronization, swapchain-image acquisition, command
         * recording, GPU submission, and presentation for the current
         * frame-in-flight slot.
         *
         * The current frame slot is reused only after its previously submitted
         * GPU work has completed.
         *
         * @throws Core::Error::EngineError if a required Vulkan frame operation
         * fails.
         *
         * @note A failed Vulkan queue submission permanently prevents
         * further frame rendering through this renderer instance.
         * Subsequent renderFrame() calls throw rather than waiting
         * on potentially unsignaled synchronization objects.
         */
        auto renderFrame() -> void;
    };
} // namespace SNE::Engine::Renderer::Vulkan
