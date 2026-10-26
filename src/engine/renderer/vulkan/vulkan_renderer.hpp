#pragma once

#include "engine/core/math/matrix4.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/camera/initial_camera_data.hpp"
#include "engine/renderer/geometry/initial_mesh_data.hpp"
#include "engine/renderer/geometry/mesh_draw_instance.hpp"
#include "engine/renderer/geometry/mesh_upload_data.hpp"
#include "engine/renderer/geometry/vertex_attribute_description.hpp"
#include "engine/renderer/material/material_buffer_binding_input.hpp"
#include "engine/renderer/material/material_description.hpp"
#include "engine/renderer/material/material_instance_description.hpp"
#include "engine/renderer/material/material_resource_binding.hpp"
#include "engine/renderer/material/material_resource_binding_set.hpp"
#include "engine/renderer/material/material_texture_binding_input.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/resource/gpu_resource_dependency_tracker.hpp"
#include "engine/renderer/resource/gpu_resource_handle.hpp"
#include "engine/renderer/resource/gpu_resource_registry.hpp"
#include "engine/renderer/shader/graphics_shader_data.hpp"
#include "engine/renderer/shader/shader_interface.hpp"
#include "engine/renderer/shader/shader_resource_type.hpp"
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
#include "engine/renderer/vulkan/vulkan_external_rendering_info.hpp"
#include "engine/renderer/vulkan/vulkan_frame_command_recorder.hpp"
#include "vulkan_frame_resources.hpp"
#include "vulkan_instance.hpp"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <span>
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
         * @brief Owns the descriptor binding for a registered texture.
         *
         * Owns the descriptor pool and stores the descriptor set allocated
         * from that pool.
         *
         * References a texture through a generation-checked resource handle.
         * The texture image, image view, and sampler remain owned by
         * m_TextureRegistry.
         *
         * The registered texture must remain alive while this binding or
         * outstanding GPU submissions reference it.
         *
         * The structure is move-only because its descriptor pool has
         * exclusive ownership.
         */
        struct TextureBindingResources {
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
         * @brief Owns descriptor allocations for a material.
         *
         * Owns the descriptor pool and stores descriptor sets allocated
         * from that pool.
         *
         * Descriptor-set positions correspond to shader descriptor-set
         * indices. The descriptor sets remain valid while their pool
         * exists and is not reset.
         *
         * The structure is move-only because its descriptor pool
         * has exclusive Vulkan ownership.
         */
        struct MaterialDescriptorResources {
            /** @brief Owns the pool backing the allocated descriptor sets. */
            VulkanDescriptorPool m_DescriptorPool;

            /**
             * @brief Descriptor-set handles allocated from m_DescriptorPool.
             *
             * The handles are non-owning and ordered by descriptor-set index.
             */
            std::vector<VkDescriptorSet> m_DescriptorSets;
        };
        static_assert(
            std::is_nothrow_move_constructible_v<MaterialDescriptorResources>);

        /**
         * @brief Stores the registered state of a material instance.
         *
         * Retains the material instance's resource binding description and
         * references its allocated descriptor resources.
         *
         * Texture and buffer resources remain owned by their respective
         * registries. Descriptor allocations remain owned by
         * m_MaterialDescriptorRegistry.
         *
         * Material instance handles identify this state independently of
         * the Vulkan descriptor resources used to render it.
         */
        struct MaterialInstanceResources {
            /**
             * @brief Resource bindings supplied when the instance was
             * registered.
             *
             * Stores non-owning handles to registered textures and buffers.
             * The renderer validates these bindings against the active material
             * interface before creating descriptor resources.
             */
            MaterialInstanceDescription m_Description{};

            /**
             * @brief Handles identifying the instance's descriptor resources.
             *
             * Contains one handle when descriptors are shared across frames,
             * or one handle per frame-in-flight slot when frame-specific
             * descriptor bindings are required.
             *
             * Handles reference m_MaterialDescriptorRegistry without owning
             * its resources.
             */
            std::vector<GpuResourceHandle> m_DescriptorHandles;
        };

        /**
         * @brief Tracks a frequently updated material buffer.
         *
         * Stores the material descriptor location, resource type, latest
         * CPU-side data, and registered GPU buffer handles for each frame slot.
         *
         * The CPU data is owned by this structure so that updates do not
         * depend on the lifetime of caller-provided memory.
         *
         * GPU buffers remain owned by m_BufferRegistry. Frame buffer handles
         * reference those resources without taking ownership.
         *
         * @note Frame-specific buffer allocation, descriptor binding, and
         * synchronization are not yet implemented.
         */
        struct FrequentMaterialBufferState {
            /**
             * @brief Tracks the GPU buffer and pending upload for one frame
             * slot.
             *
             * The buffer is owned by m_BufferRegistry and identified through
             * a non-owning resource handle.
             *
             * The upload flag indicates whether the buffer needs to receive
             * the latest CPU-side data before its next GPU submission.
             *
             * The buffer must only be modified after GPU work previously
             * submitted through the corresponding frame slot has completed.
             */
            struct FrameState {
                /** @brief Registered GPU buffer belonging to this frame slot.
                 */
                GpuResourceHandle m_BufferHandle{};

                /** @brief Whether the frame slot requires the latest CPU data.
                 */
                bool m_NeedsUpload{};
            };

            /** @brief Descriptor-set index containing the material buffer. */
            std::uint32_t m_Set{};

            /** @brief Binding index within the descriptor set. */
            std::uint32_t m_Binding{};

            /** @brief Element index within the descriptor binding array. */
            std::uint32_t m_ArrayElement{};

            /** @brief Shader resource type of the material buffer. */
            Shader::ShaderResourceType m_Type{
                Shader::ShaderResourceType::UniformBuffer,
            };

            /** @brief Owns the latest CPU-side data requested for the buffer.
             */
            std::vector<std::byte> m_LatestData;

            /**
             * @brief Per-frame buffer handles and pending upload states.
             *
             * Entries are indexed by frame-in-flight slot.
             *
             * Once initialized, the number of entries must match
             * m_FrameResources.size().
             */
            std::vector<FrameState> m_Frames;
        };

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
         * @brief Stores the reflected interfaces of a graphics shader pair.
         *
         * Retains the vertex and fragment shader interfaces used during
         * initial material validation and descriptor layout generation.
         */
        struct ReflectedGraphicsShaderInterfaces {
            /** @brief Reflected vertex shader interface. */
            Shader::ShaderInterface vertex;

            /** @brief Reflected fragment shader interface. */
            Shader::ShaderInterface fragment;
        };

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
         * @brief Whether swapchain images require transfer-source support.
         *
         * Preserves the framebuffer-readback requirement across initial
         * swapchain creation and subsequent recreation.
         *
         * Defaults to false to preserve normal presentation behavior.
         */
        bool m_RequireSwapchainTransferSource{false};

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
         * @brief Owns descriptor-set layouts generated from shader
         * requirements.
         *
         * Layout positions correspond to descriptor-set indices.
         * Empty descriptor sets have valid layouts with no bindings.
         *
         * These layouts must outlive cached graphics pipelines that reference
         * them.
         */
        std::vector<VulkanDescriptorSetLayout> m_DescriptorSetLayouts;

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
         * @brief Owns the renderer's registered standalone GPU buffers.
         *
         * Stores Vulkan buffers and their VMA-backed allocations using
         * generation-checked, registry-scoped resource handles.
         *
         * Material bindings reference registered buffers without taking
         * ownership of their GPU resources.
         *
         * Buffers referenced by outstanding GPU submissions must remain
         * alive until the corresponding GPU work has completed.
         *
         * Resources removed while GPU work is in flight must be retained
         * through the renderer's resource retirement mechanism.
         */
        GpuResourceRegistry<VulkanBuffer> m_BufferRegistry;

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
         * @brief Tracks dependencies between renderer-managed GPU resources.
         *
         * Stores non-owning relationships between resource identities.
         * The tracked relationships will be used to validate resource
         * replacement and removal.
         *
         * Resource ownership remains with the corresponding registries.
         * GPU submission completion and deferred destruction are handled
         * separately by the resource retirement queue.
         *
         * @note The tracker does not validate resource existence or own
         * referenced GPU resources.
         */
        GpuResourceDependencyTracker m_ResourceDependencies;

        /**
         * @brief Owns the active sampled-texture descriptor binding.
         *
         * Stores the descriptor pool, allocated descriptor set, and handle
         * identifying the registered texture referenced by the binding.
         *
         * The texture's GPU resources are owned separately by
         * m_TextureRegistry.
         *
         * Replaced descriptor bindings and their referenced textures are
         * retired together until outstanding GPU submissions complete.
         *
         * Empty when no texture binding has been initialized.
         */
        std::optional<TextureBindingResources> m_TextureBindingResources;

        /**
         * @brief Owns registered material descriptor resources.
         *
         * Stores descriptor pools and their allocated descriptor sets
         * using generation-checked, registry-scoped handles.
         *
         * Descriptor resources may depend on registered textures and
         * buffers without owning those resources.
         *
         * Referenced GPU resources must remain alive while descriptor
         * resources or outstanding GPU submissions require them.
         *
         * @note Resource dependency tracking does not replace GPU
         * completion-aware retirement.
         */
        GpuResourceRegistry<MaterialDescriptorResources>
            m_MaterialDescriptorRegistry;

        /**
         * @brief Owns registered material instance states.
         *
         * Stores material instance descriptions and non-owning handles to
         * their descriptor resources.
         *
         * Material instances are identified by generation-checked,
         * registry-scoped GPU resource handles.
         *
         * Descriptor allocations, textures, and buffers remain owned
         * by their respective registries.
         *
         * Material instance removal must preserve resources referenced
         * by outstanding GPU submissions until execution completes.
         */
        GpuResourceRegistry<MaterialInstanceResources>
            m_MaterialInstanceRegistry;

        /**
         * @brief Identifies the active material descriptor resources.
         *
         * References descriptor resources owned by
         * m_MaterialDescriptorRegistry without taking ownership.
         *
         * Empty when no active material descriptor resources exist.
         *
         * The handle must resolve to registered descriptor resources
         * before their descriptor sets are used for rendering.
         *
         * @note Replacing the active handle does not destroy the
         * previously selected descriptor resources.
         */
        std::optional<GpuResourceHandle> m_ActiveMaterialDescriptorHandle;

        /**
         * @brief Identifies material descriptor resources for individual frame
         * slots.
         *
         * Each handle references descriptor resources owned by
         * m_MaterialDescriptorRegistry.
         *
         * Entries are ordered by frame-in-flight slot. Each frame's descriptor
         * sets will reference the corresponding frequently updated GPU buffers,
         * along with any shared material resources.
         *
         * Empty when frame-specific material descriptors are not configured.
         *
         * @note Descriptor creation, selection, and GPU lifetime management
         * for frequent material updates are not yet implemented.
         */
        std::vector<GpuResourceHandle> m_FrameMaterialDescriptorHandles;

        /**
         * @brief Stores descriptor counts required by the current material.
         *
         * Reuses the shader-derived descriptor counts when allocating
         * replacement material descriptor pools.
         */
        std::vector<VkDescriptorPoolSize> m_MaterialDescriptorPoolSizes;

        /**
         * @brief Stores the material resource bindings used for rendering.
         *
         * Owns backend-independent binding descriptions containing descriptor
         * locations, resource types, and non-owning GPU resource handles.
         *
         * Referenced GPU resources remain owned by their resource registries.
         * Vulkan descriptor allocations are owned separately.
         *
         * An empty collection indicates that no material resource bindings
         * have been configured.
         *
         * The renderer supports combined image samplers, uniform buffers,
         * and storage buffers at shader-declared descriptor locations.
         */
        MaterialResourceBindingSet m_MaterialBindings;

        /**
         * @brief Stores frequently updated material-buffer states.
         *
         * Each entry tracks one material descriptor location, the latest
         * CPU-side data, and GPU buffer handles associated with frame slots.
         *
         * GPU buffer ownership remains with m_BufferRegistry.
         *
         * Empty when no frequently updated material buffers are configured.
         *
         * @note Frequent material-buffer initialization and updates are not
         * yet implemented.
         */
        std::vector<FrequentMaterialBufferState> m_FrequentMaterialBuffers;

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
         * @brief Stores the mesh instances submitted for rendering.
         *
         * Associates non-owning mesh resource handles with model
         * transformations. Mesh GPU resources remain owned by m_MeshRegistry.
         *
         * Instance data is independent of Vulkan resource ownership and does
         * not represent persistent scene or game-object identity.
         *
         * Empty when no mesh instances have been configured.
         */
        std::vector<MeshDrawInstance> m_DrawInstances;

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
         * @brief Creates depth attachments for the frame-in-flight slots.
         *
         * Creates one depth image and image view per frame slot using
         * the requested extent and depth format.
         *
         * @param extent Extent of the replacement swapchain.
         * @param depth_format Supported Vulkan depth attachment format.
         *
         * @pre extent must have nonzero dimensions.
         * @pre depth_format must be a supported depth attachment format.
         *
         * @return Owned depth attachment resources.
         *
         * @post The returned collection contains one depth attachment
         * per frame-in-flight slot.
         *
         * @throws Core::Error::EngineError if Vulkan resource creation fails.
         * @throws std::bad_alloc if resource storage allocation fails.
         */
        [[nodiscard]] auto createDepthAttachments(VkExtent2D extent,
                                                  VkFormat depth_format)
            -> std::vector<DepthAttachmentResources>;

        /**
         * @brief Finds or creates a graphics pipeline for a material.
         *
         * Uses the supplied material configuration and current mesh,
         * descriptor-set layouts, and attachment formats.
         *
         * @param material Material configuration used to select the pipeline.
         * @param swapchain Swapchain providing the color attachment format.
         * @param depth_format Depth attachment format.
         *
         * @pre An active mesh must exist and its vertex layout must be
         * initialized.
         *
         * @return Non-owning pointer to the selected graphics pipeline, owned
         * by m_GraphicsPipelineCache.
         *
         * @throws Core::Error::EngineError if pipeline creation fails.
         * @throws std::bad_alloc if host resource allocation fails.
         */
        [[nodiscard]] auto
        getOrCreateGraphicsPipeline(const MaterialDescription &material,
                                    const VulkanSwapchain &swapchain,
                                    VkFormat depth_format)
            -> const VulkanGraphicsPipeline *;

        /**
         * @brief Selects the graphics pipeline for a replacement swapchain.
         *
         * Creates or retrieves a compatible graphics pipeline when the
         * attachment formats require one.
         *
         * Reuses the previous pipeline when its configuration remains
         * compatible with the replacement swapchain.
         *
         * @param swapchain Replacement swapchain.
         * @param depth_format Selected depth attachment format.
         *
         * @return Non-owning pointer to the selected graphics pipeline,
         * or nullptr when no pipeline is available or required.
         *
         * @pre A material must exist when pipeline creation is required.
         *
         * @note Returned pipelines are owned by m_GraphicsPipelineCache.
         *
         * @throws Core::Error::EngineError if pipeline creation fails.
         */
        [[nodiscard]] auto
        selectGraphicsPipeline(const VulkanSwapchain &swapchain,
                               VkFormat depth_format)
            -> const VulkanGraphicsPipeline *;

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
         * @brief Creates GPU buffers for supplied mesh geometry.
         *
         * Validates borrowed vertex and index data, creates device-local
         * vertex and index buffers, and uploads the supplied contents.
         *
         * Does not register the mesh or modify the active mesh selection.
         *
         * @param mesh Borrowed mesh geometry to upload.
         * @return Owned GPU mesh buffer resources.
         *
         * @throws Core::Error::EngineError if GPU allocation or upload fails.
         *
         * @note Invalid mesh data triggers precondition assertions.
         */
        [[nodiscard]] auto createMeshBufferResources(const MeshUploadData &mesh)
            -> MeshBufferResources;

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
         * @post On successful replacement, draw instances referencing the
         * previous mesh are updated to reference the replacement mesh.
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
         * @param record_external_commands Optional callback that records
         * additional graphics commands into the active command buffer after
         * mesh drawing and before dynamic rendering ends. An empty callback
         * skips external rendering.
         *
         * @pre The callback must not end the active rendering scope, submit or
         * reset the command buffer, or retain it beyond the invocation.
         * @pre The callback must not throw exceptions.
         * @pre command_buffer must be valid and ready for recording.
         * @pre image_index must reference an image in swapchain_resources.
         *
         * @throws Core::Error::EngineError if command buffer recording
         * initialization or completion fails.
         */
        auto recordFrameCommands(
            VkCommandBuffer command_buffer,
            const SwapchainResources &swapchain_resources,
            const DepthAttachmentResources &depth_attachment,
            std::size_t image_index,
            const VulkanFrameCommandRecorder &record_external_commands) -> void;

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
         * @brief Binds the renderer's default material descriptors.
         *
         * Selects the descriptor resources for the current frame and binds
         * them to the supplied graphics pipeline layout.
         *
         * Falls back to the legacy texture descriptor binding when no
         * registered default material descriptors are available.
         *
         * @param command_buffer Command buffer recording graphics commands.
         * @param pipeline_layout Graphics pipeline layout used for binding.
         *
         * @pre Both Vulkan handles must be valid for command recording.
         *
         * @note Does not own or allocate descriptor resources.
         */
        auto bindMaterialDescriptors(
            VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout,
            std::optional<GpuResourceHandle> material_handle) const -> void;

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
         * @pre The mip-level count must be at least one and must not exceed
         * the maximum supported by the texture dimensions.
         * @pre Pixel storage must contain the complete, tightly packed RGBA8
         * mip chain, with four bytes per texel across all supplied levels.
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

        /**
         * @brief Uploads and registers a sampled material texture.
         *
         * Creates GPU texture resources from the supplied upload description
         * and transfers ownership to the texture registry.
         *
         * Returns a material binding associating the registered texture with
         * the requested shader descriptor location.
         *
         * @param input Texture upload data and descriptor location.
         *
         * @return Non-owning material binding referencing the registered
         * texture.
         *
         * @post On success, the returned handle resolves to a texture owned
         * by m_TextureRegistry.
         *
         * @throws Core::Error::EngineError if GPU allocation or upload fails.
         * @throws std::bad_alloc if resource registration fails.
         *
         * @note The input's pixel storage is borrowed until upload completes.
         * @note Descriptor allocation and material selection are not performed.
         */
        [[nodiscard]] auto
        registerMaterialTextureBinding(const MaterialTextureBindingInput &input)
            -> MaterialResourceBinding;

        /**
         * @brief Creates a descriptor binding for a registered texture.
         *
         * Validates the material resource binding against the supported
         * sampled-texture descriptor location and resource type.
         *
         * Resolves the resource handle through m_TextureRegistry, allocates
         * a descriptor pool and descriptor set, and writes the registered
         * texture's image view and sampler into the descriptor set.
         *
         * The returned binding owns its descriptor pool but does not take
         * ownership of the registered texture.
         *
         * @param resource_binding Material resource binding identifying the
         * descriptor location, resource type, and registered texture.
         * @param layout Descriptor-set layout used to allocate the binding.
         *
         * @pre resource_binding must specify descriptor set zero, binding zero,
         * array element zero, and
         * Shader::ShaderResourceType::CombinedImageSampler.
         * @pre resource_binding.resource_handle must resolve to a registered
         * texture in m_TextureRegistry.
         * @pre layout must reference a valid, compatible descriptor-set layout.
         *
         * @return Owned descriptor binding referencing the registered texture.
         *
         * @post The registered texture remains owned by m_TextureRegistry.
         *
         * @throws Core::Error::EngineError if descriptor pool creation
         * or descriptor-set allocation fails.
         * @throws std::bad_alloc if host allocation fails.
         *
         * @note Invalid material resource bindings trigger precondition
         * assertions.
         *
         * @note The registered texture must outlive the descriptor binding
         * and all GPU submissions referencing it.
         */
        [[nodiscard]] auto createTextureBindingResources(
            const MaterialResourceBinding &resource_binding,
            const VulkanDescriptorSetLayout &layout) -> TextureBindingResources;

        /**
         * @brief Registers replacement material descriptors for a texture.
         *
         * Copies the current material bindings, substitutes the replacement
         * texture binding, and registers descriptor resources when
         * shader-driven material descriptors are active.
         *
         * @param replacement_binding Binding referencing the replacement
         * texture.
         * @param has_active_material_descriptors Whether material descriptors
         * are currently active.
         *
         * @return Handle of the registered replacement descriptors, or
         * std::nullopt when no material descriptors are active.
         *
         * @post Does not change the active material descriptor handle.
         * @post On failure, newly registered descriptor resources and
         * dependency relationships are rolled back.
         */
        [[nodiscard]] auto registerTextureReplacementMaterialDescriptors(
            const MaterialResourceBinding &replacement_binding,
            bool has_active_material_descriptors)
            -> std::optional<GpuResourceHandle>;

        /**
         * @brief Retires resources replaced by a texture update.
         *
         * Retires the previous texture only when it has no remaining
         * dependents. Always retires the previous standalone texture
         * binding and any active material descriptor resources.
         *
         * @param previous_texture Previously registered texture resources.
         * @param previous_binding Previous standalone texture binding.
         * @param retire_texture Whether the previous texture can be retired.
         *
         * @post On success, ownership of selected resources is transferred
         * to the graphics retirement queue.
         * @post If retirement preparation throws, resource ownership
         * remains unchanged.
         *
         * @throws std::bad_alloc if retirement preparation fails.
         *
         * @note Call only after replacement resources have been prepared.
         * @note Does not update registries, dependency relationships,
         * active handles, or material bindings.
         */
        auto retireTextureReplacementResources(
            TextureResources &previous_texture,
            TextureBindingResources &previous_binding, bool retire_texture)
            -> void;

        /**
         * @brief Validates a material binding against its descriptor layout.
         *
         * Checks that the descriptor-set index is valid, the binding exists
         * in the corresponding layout, and its descriptor type and array
         * element match the layout declaration.
         *
         * @param binding Material resource binding to validate.
         * @param descriptor_set_count Number of descriptor sets allocated
         * for the destination resources.
         *
         * @pre descriptor_set_count describes the destination descriptor sets.
         *
         * @note Invalid binding metadata triggers a Vulkan precondition
         * assertion.
         */
        auto validateMaterialDescriptorBinding(
            const MaterialResourceBinding &binding,
            std::size_t descriptor_set_count) const -> void;

        /**
         * @brief Validates a material instance description for registration.
         *
         * Checks that an active material exists, the supplied bindings satisfy
         * the vertex and fragment shader interfaces, and the descriptor
         * locations and referenced GPU resources are valid.
         *
         * Also validates compatibility with the renderer's frequently
         * updated material-buffer configuration.
         *
         * @param description Material instance description to validate.
         *
         * @pre The renderer must have an initialized material configuration.
         * @pre Resource bindings must satisfy the active shader interfaces.
         * @pre Referenced resources must be registered and compatible.
         *
         * @post Renderer resources and material state remain unchanged.
         *
         * @throws Core::Error::EngineError if shader reflection fails.
         *
         * @note Invalid descriptions trigger precondition assertions.
         * @note This operation does not allocate descriptor resources.
         */
        auto validateMaterialInstanceDescription(
            const MaterialInstanceDescription &description) const -> void;

        /**
         * @brief Validates one material instance resource binding.
         *
         * Checks the descriptor location and resource type against the
         * active descriptor-set layouts, then verifies that the referenced
         * texture or buffer exists in the corresponding resource registry.
         *
         * @param binding Material resource binding to validate.
         *
         * @pre Descriptor-set layouts must describe the active shaders.
         * @pre The binding must reference a supported resource type.
         * @pre Buffer resources must have Vulkan usage flags compatible
         * with their declared shader resource types.
         *
         * @post Renderer resources remain unchanged.
         *
         * @note Invalid bindings and resource handles trigger
         * precondition assertions.
         */
        auto validateMaterialInstanceResourceBinding(
            const MaterialResourceBinding &binding) const -> void;

        /**
         * @brief Validates shared frequently updated material-buffer bindings.
         *
         * Ensures that each renderer-managed frequent material buffer has
         * a corresponding binding with the expected descriptor location,
         * resource type, and initial frame-buffer handle.
         *
         * @param bindings Material resource bindings to validate.
         *
         * @pre Frequently updated material buffers must have initialized
         * frame-specific GPU resources.
         *
         * @post Renderer resources remain unchanged.
         *
         * @note Invalid shared bindings trigger precondition assertions.
         * @note Incomplete internal frequent-buffer state triggers an
         * invariant assertion.
         */
        auto validateMaterialInstanceFrequentBindings(
            std::span<const MaterialResourceBinding> bindings) const -> void;

        /**
         * @brief Prepares a descriptor write for a sampled texture.
         *
         * Resolves the texture handle through the texture registry and
         * appends its sampler and image view information to the supplied
         * storage.
         *
         * @param binding Material binding identifying the sampled texture.
         * @param write Descriptor write being prepared.
         * @param image_infos Storage retaining descriptor image information.
         *
         * @pre binding must identify a registered sampled texture.
         * @pre image_infos must have sufficient reserved capacity to keep
         * its elements stable until descriptor updates are issued.
         *
         * @post write.pImageInfo points to the appended image information.
         *
         * @note This function does not issue descriptor updates.
         */
        auto writeMaterialTextureDescriptor(
            const MaterialResourceBinding &binding, VkWriteDescriptorSet &write,
            std::vector<VkDescriptorImageInfo> &image_infos) const -> void;

        /**
         * @brief Prepares a descriptor write for a registered buffer.
         *
         * Resolves the buffer handle through m_BufferRegistry and validates
         * the buffer size against the device limit for the requested
         * descriptor type.
         *
         * Appends the buffer information to the supplied storage and assigns
         * it to the descriptor write.
         *
         * @param binding Material binding identifying the registered buffer
         * and its descriptor type.
         * @param write Descriptor write being prepared.
         * @param buffer_infos Storage retaining descriptor buffer information.
         *
         * @pre binding.type must be UniformBuffer or StorageBuffer.
         * @pre binding.resource_handle must resolve to a registered buffer.
         * @pre buffer_infos must have sufficient reserved capacity to keep
         * its elements stable until descriptor updates are issued.
         *
         * @post write.pBufferInfo points to the appended buffer information.
         *
         * @note This function does not issue descriptor updates.
         */
        auto writeMaterialBufferDescriptor(
            const MaterialResourceBinding &binding, VkWriteDescriptorSet &write,
            std::vector<VkDescriptorBufferInfo> &buffer_infos) const -> void;

        /**
         * @brief Writes material resources into the supplied Vulkan descriptor
         * sets.
         *
         * Resolves material resource handles through their respective
         * registries and updates the destination descriptor sets according
         * to the supplied material bindings.
         *
         * @param destination Descriptor pool and descriptor sets receiving
         * the resource bindings.
         * @param bindings Material resource bindings to write into the
         * destination descriptor sets.
         *
         * @pre The destination descriptor sets must already be allocated.
         * @pre The supplied bindings must reference registered resources
         * compatible with the corresponding descriptor declarations.
         * @pre Referenced GPU resources must remain alive while the descriptor
         * sets may be used by GPU submissions.
         * @pre The destination descriptor sets must not be in use by pending
         * GPU submissions when updated without the required Vulkan
         * update-after-bind support.
         *
         * @note This function does not allocate descriptor sets or transfer
         * ownership of GPU resources.
         */
        auto writeMaterialDescriptorSets(
            MaterialDescriptorResources &destination,
            std::span<const MaterialResourceBinding> bindings) -> void;

        /**
         * @brief Writes registered material resources into Vulkan descriptor
         * sets.
         *
         * Resolves material resource handles through their GPU resource
         * registries and updates the corresponding shader-defined descriptor
         * locations.
         *
         * Descriptor sets are owned by material descriptor resources stored in
         * m_MaterialDescriptorRegistry. Referenced textures and buffers remain
         * owned by their respective resource registries.
         *
         * @pre Material descriptor sets must already be allocated.
         * @pre Material bindings must reference registered, compatible
         * resources.
         * @pre Descriptor sets must not be in use by pending GPU submissions
         * when updated without the required Vulkan update-after-bind support.
         *
         * @note This function does not allocate descriptor sets or transfer
         * ownership of GPU resources.
         */
        auto updateMaterialDescriptorSets() -> void;

        /**
         * @brief Finds the currently active material descriptor resources.
         *
         * Resolves the active descriptor handle through the material
         * descriptor registry when one is selected.
         *
         * During migration, falls back to the existing optional descriptor
         * resources when no registry handle is active.
         *
         * @return Pointer to the active descriptor resources, or nullptr
         * if no descriptor resources have been allocated.
         *
         * @note An active handle that fails to resolve is an invariant
         * violation and must not trigger the legacy fallback.
         *
         * @note Does not transfer ownership or retire GPU resources.
         */
        [[nodiscard]] auto findActiveMaterialDescriptorResources() noexcept
            -> MaterialDescriptorResources *;

        /**
         * @brief Finds the currently active material descriptor resources.
         *
         * Resolves the active descriptor handle through the registry.
         * During migration, falls back to the legacy optional when no
         * active registry handle exists.
         *
         * @return Const pointer to the active descriptor resources, or
         * nullptr if no descriptor resources have been allocated.
         *
         * @note An active handle that fails to resolve triggers an
         * invariant assertion.
         *
         * @note Does not modify or transfer ownership of resources.
         */
        [[nodiscard]] auto
        findActiveMaterialDescriptorResources() const noexcept
            -> const MaterialDescriptorResources *;

        /**
         * @brief Finds the material descriptor resources used by a frame slot.
         *
         * Selects frame-specific material descriptors when configured.
         * Otherwise, uses the existing active material descriptor resources.
         *
         * Descriptor resources remain owned by m_MaterialDescriptorRegistry.
         *
         * @param frame_index Index of the frame-in-flight slot.
         *
         * @return Pointer to the selected descriptor resources, or nullptr
         * when no material descriptor resources are configured.
         *
         * @pre frame_index must identify a valid frame slot.
         *
         * @note Invalid frame-specific descriptor state triggers an
         * invariant assertion.
         *
         * @note Does not allocate, modify, or transfer GPU resources.
         */
        [[nodiscard]] auto findMaterialDescriptorResourcesForFrame(
            std::size_t frame_index) const noexcept
            -> const MaterialDescriptorResources *;

        /**
         * @brief Finds descriptor resources for a material instance and frame.
         *
         * Selects shared descriptor resources or frame-specific descriptor
         * resources associated with the material instance.
         *
         * @param material_handle Registered material instance handle.
         * @param frame_index Current frame-in-flight index.
         * @return Descriptor resources for the selected frame, or nullptr
         * when the material requires no descriptor sets.
         *
         * @pre material_handle must resolve to a registered material instance.
         * @pre frame_index must identify a valid frame slot.
         *
         * @note Invalid material or descriptor registry state triggers an
         * invariant assertion.
         * @note Does not allocate resources or modify descriptor bindings.
         */
        [[nodiscard]] auto findMaterialInstanceDescriptorResourcesForFrame(
            GpuResourceHandle material_handle,
            std::size_t frame_index) const noexcept
            -> const MaterialDescriptorResources *;

        /**
         * @brief Creates registered GPU buffers for a frequently updated
         * material binding.
         *
         * Copies the supplied initial data into renderer-owned CPU storage and
         * creates one initialized GPU buffer for each frame-in-flight slot.
         *
         * GPU buffers are registered with m_BufferRegistry. The returned state
         * stores non-owning resource handles and tracks pending uploads for
         * individual frame slots.
         *
         * All frame buffers are initialized with the same CPU data, so their
         * pending-upload flags are initially false.
         *
         * @param input Material buffer description containing its descriptor
         * location, resource type, update frequency, and initial CPU data.
         *
         * @return Frequent material-buffer state containing owned CPU data and
         * registered buffer handles ordered by frame slot.
         *
         * @pre input.update_frequency must be Frequent.
         * @pre input.type must be UniformBuffer or StorageBuffer.
         * @pre input.data must not be empty.
         * @pre Frame resources must already be initialized.
         *
         * @post On success, the number of frame states matches
         * m_FrameResources.size().
         * @post Every frame state references an initialized GPU buffer owned
         * by m_BufferRegistry.
         * @post All pending-upload flags are false.
         * @post On failure, all buffers registered during this operation
         * are removed, leaving no partially registered resources.
         *
         * @throws Core::Error::EngineError if GPU buffer creation, mapping,
         * or memory flushing fails.
         * @throws std::bad_alloc if CPU storage allocation or resource
         * registration fails.
         *
         * @note Newly registered buffers are not referenced by descriptor sets
         * or outstanding GPU submissions during this operation. Failed
         * initialization can therefore release them immediately.
         *
         * @note This function does not create descriptor resources, select
         * active material bindings, or submit GPU work.
         */
        [[nodiscard]] auto createFrequentMaterialBufferState(
            const MaterialBufferBindingInput &input)
            -> FrequentMaterialBufferState;

        /**
         * @brief Removes GPU buffers registered for a frequent material buffer.
         *
         * Rolls back the GPU buffer registrations associated with the supplied
         * frequent material-buffer state during initialization.
         *
         * Each frame-specific buffer is removed from m_BufferRegistry,
         * releasing its owned Vulkan buffer and VMA allocation.
         *
         * The supplied state owns CPU-side data and stores non-owning GPU
         * resource handles. This function does not modify the state itself.
         *
         * @param state Frequent material-buffer state whose registered GPU
         * buffers are being rolled back.
         *
         * @pre Every frame buffer handle must identify a resource registered
         * with m_BufferRegistry.
         * @pre No descriptor resources or outstanding GPU submissions may
         * reference the buffers being removed.
         *
         * @post All registered GPU buffers referenced by the supplied frame
         * states have been removed from m_BufferRegistry.
         * @post The corresponding resource handles no longer resolve.
         * @post The supplied CPU-side state remains unchanged.
         *
         * @note This function is intended only for initialization rollback.
         * Active GPU resources must instead use completion-aware retirement.
         *
         * @note This function does not remove descriptor resources, modify
         * material bindings, or synchronize GPU execution.
         */
        auto rollbackFrequentMaterialBufferState(
            const FrequentMaterialBufferState &state) -> void;

        /**
         * @brief Stages updated CPU data for a frequently updated material
         * buffer.
         *
         * Validates and copies the supplied data into the buffer state's owned
         * CPU storage, then marks every frame slot as requiring an upload.
         *
         * GPU buffer contents are not modified immediately. Pending uploads
         * are processed when the corresponding frame slot is safe to reuse
         * after its previous GPU work has completed.
         *
         * Multiple updates before a frame slot is processed are combined,
         * so that only the latest CPU data needs to be uploaded.
         *
         * @param state Frequent material-buffer state to update.
         * @param data Borrowed CPU data containing the replacement contents.
         *
         * @pre data must not be empty.
         * @pre data.size_bytes() must equal state.m_LatestData.size().
         * @pre The operation must be serialized with rendering and other
         * modifications to the material-buffer state.
         *
         * @post On success, state.m_LatestData contains a copy of data.
         * @post Every frame state's m_NeedsUpload flag is true.
         * @post On failure, the previous CPU data and pending-upload flags
         * remain unchanged.
         *
         * @throws std::bad_alloc if copying the replacement data requires
         * an allocation that fails.
         *
         * @note This function does not allocate, replace, or write GPU buffers.
         * It does not modify descriptor resources or submit GPU work.
         *
         * @note The caller's data only needs to remain valid until this
         * operation completes.
         */
        static auto
        stageFrequentMaterialBufferUpdate(FrequentMaterialBufferState &state,
                                          std::span<const std::byte> data)
            -> void;

        /**
         * @brief Uploads pending material-buffer updates for a frame slot.
         *
         * Iterates over frequently updated material buffers and writes the
         * latest CPU-side data into the registered GPU buffer belonging to the
         * specified frame slot.
         *
         * Buffers without pending updates are skipped. Each frame's
         * pending-upload flag is cleared only after its buffer write completes
         * successfully.
         *
         * @param frame_index Index of the frame-in-flight slot to update.
         *
         * @pre frame_index must be less than m_FrameResources.size().
         * @pre Every frequent material-buffer state must contain a frame entry
         * for the requested index.
         * @pre Every pending frame buffer handle must resolve to a registered
         * upload buffer in m_BufferRegistry.
         * @pre The latest CPU data must match the corresponding GPU buffer
         * size.
         * @pre Previous GPU work using the specified frame slot must have
         * completed before this function is called.
         * @pre The operation must be serialized with rendering and other
         * modifications to material-buffer state.
         *
         * @post On success, all pending updates for the specified frame slot
         * have been uploaded and their pending-upload flags are false.
         * @post Other frame slots and their pending-upload flags remain
         * unchanged.
         * @post If an upload fails, its pending-upload flag remains true.
         *
         * @throws Core::Error::EngineError if GPU memory mapping or flushing
         * fails.
         *
         * @note This function reuses existing GPU buffers and does not allocate
         * replacement buffers, modify descriptor sets, or submit GPU work.
         *
         * @note The caller is responsible for GPU synchronization. This
         * function does not wait for fences or establish synchronization
         * itself.
         *
         * @note If an upload fails after earlier buffers were updated, those
         * successful uploads remain committed. The operation is not atomic
         * across multiple material buffers.
         */
        auto uploadFrequentMaterialBuffersForFrame(std::size_t frame_index)
            -> void;

        /**
         * @brief Creates a material binding for a registered buffer.
         *
         * Registers a uniform buffer initialized with the supplied CPU data
         * and associates its resource handle with the requested shader
         * descriptor location.
         *
         * The returned binding does not own the registered buffer.
         *
         * @param input Initial buffer data and shader binding information.
         *
         * @pre input.type must identify a uniform buffer or storage buffer.
         * @pre input.data must not be empty.
         *
         * @return Material binding referencing the registered uniform buffer.
         *
         * @post The returned resource handle resolves to a buffer owned
         * by m_BufferRegistry.
         *
         * @throws Core::Error::EngineError if buffer creation or writing fails.
         * @throws std::bad_alloc if resource registration fails.
         */
        [[nodiscard]] auto
        registerMaterialBufferBinding(const MaterialBufferBindingInput &input)
            -> MaterialResourceBinding;

        /**
         * @brief Registers a frequently updated material-buffer binding.
         *
         * Creates initialized GPU buffers for each frame-in-flight slot and
         * stores their tracking state in m_FrequentMaterialBuffers.
         *
         * Returns a material binding referencing the first frame's buffer.
         * Frame-specific descriptor creation later substitutes the appropriate
         * buffer handle for each frame slot.
         *
         * @param input Initial material-buffer binding configuration.
         *
         * @return Material binding referencing the first frame's GPU buffer.
         *
         * @pre input must describe a valid frequently updated material buffer.
         * @pre Frame resources must already be initialized.
         *
         * @post On success, one frequent material-buffer state is registered.
         * @post On failure, buffers created by this operation are rolled back.
         *
         * @throws Core::Error::EngineError if GPU resource creation fails.
         * @throws std::bad_alloc if resource allocation fails.
         *
         * @note Does not create descriptor sets or submit GPU work.
         */
        [[nodiscard]] auto registerFrequentMaterialBufferBinding(
            const MaterialBufferBindingInput &input) -> MaterialResourceBinding;

        /**
         * @brief Initializes the renderer's frame-in-flight resources.
         *
         * Creates one VulkanFrameResources instance for each frame-in-flight
         * slot using the selected graphics queue family.
         *
         * The frame resources remain owned by m_FrameResources.
         *
         * @post m_FrameResources contains kFramesInFlight initialized
         * frame-resource instances.
         *
         * @throws Core::Error::EngineError if Vulkan resource creation fails.
         * @throws std::bad_alloc if resource storage allocation fails.
         */
        auto initializeFrameResources() -> void;

        /**
         * @brief Initializes the renderer's initial mesh.
         *
         * Copies the supplied vertex layout and model transformations into
         * renderer-owned storage, then uploads and registers the mesh.
         *
         * The supplied CPU data is borrowed only during initialization.
         *
         * @param mesh Initial mesh data, vertex layout, and model matrices.
         *
         * @post On success, the renderer owns the mesh's GPU resources and
         * retains its vertex layout and model transformations.
         *
         * @throws Core::Error::EngineError if GPU resource creation or
         * upload fails.
         * @throws std::bad_alloc if resource storage allocation fails.
         *
         * @note Invalid mesh data triggers precondition assertions.
         */
        auto initializeInitialMesh(const InitialMeshData &mesh) -> void;

        /**
         * @brief Initializes the renderer's graphics material.
         *
         * Transfers ownership of the supplied shaders to m_Material,
         * reflects their interfaces, and creates the descriptor-set
         * layouts and initial descriptor allocations.
         *
         * @param shaders Graphics shaders whose ownership is transferred.
         *
         * @return Reflected vertex and fragment shader interfaces.
         *
         * @post On success, m_Material contains the supplied shaders
         * and descriptor resources have been initialized when required.
         *
         * @throws Core::Error::EngineError if shader reflection or
         * Vulkan descriptor resource creation fails.
         * @throws std::bad_alloc if resource allocation fails.
         */
        [[nodiscard]] auto
        initializeInitialMaterial(Shader::GraphicsShaderData shaders)
            -> ReflectedGraphicsShaderInterfaces;

        /**
         * @brief Allocates descriptor resources for the current material.
         *
         * Creates a descriptor pool using the supplied descriptor counts
         * and allocates descriptor sets from m_DescriptorSetLayouts.
         *
         * Does not replace the currently active material descriptor
         * resources.
         *
         * @param pool_sizes Descriptor counts required by the material.
         *
         * @pre m_DescriptorSetLayouts must not be empty.
         *
         * @return Owned descriptor pool and allocated descriptor sets.
         *
         * @throws Core::Error::EngineError if Vulkan descriptor resource
         * creation or allocation fails.
         * @throws std::bad_alloc if host allocation fails.
         *
         * @note The caller is responsible for installing or retiring
         * the returned resources.
         */
        [[nodiscard]] auto createMaterialDescriptorResources(
            std::span<const VkDescriptorPoolSize> pool_sizes)
            -> MaterialDescriptorResources;

        /**
         * @brief Builds material resource bindings for a specific frame slot.
         *
         * Copies the renderer's current material bindings and substitutes
         * frame-specific GPU buffer handles for frequently updated material
         * buffers.
         *
         * Shared material resources, such as textures and occasionally updated
         * buffers, retain their existing resource handles.
         *
         * The returned bindings are independent of m_MaterialBindings and
         * reference resources owned by their respective GPU resource
         * registries.
         *
         * @param frame_index Index of the frame-in-flight slot whose material
         * bindings are being prepared.
         * @param base_bindings Material bindings to use as the starting point.
         * The supplied bindings are copied before frame-specific buffer handles
         * are substituted.
         *
         * @return Material resource bindings referencing the GPU resources
         * required by the specified frame slot.
         *
         * @pre frame_index must be less than m_FrameResources.size().
         * @pre Every frequently updated material buffer must have a valid
         * registered buffer handle for the requested frame slot once
         * frequent-buffer initialization is enabled.
         *
         * @post The original m_MaterialBindings remains unchanged.
         * @post Shared resource handles are preserved.
         * @post Frequently updated buffer bindings reference the corresponding
         * frame-specific GPU buffers once substitution is implemented.
         *
         * @throws std::bad_alloc if copying the material bindings fails.
         *
         * @note This function does not allocate GPU resources, create
         * descriptor sets, modify registered resources, or submit GPU work.
         *
         * @note Frame-specific buffer substitution is not yet implemented.
         */
        [[nodiscard]] auto buildFrameMaterialBindings(
            std::size_t frame_index,
            std::span<const MaterialResourceBinding> base_bindings) const
            -> std::vector<MaterialResourceBinding>;

        /**
         * @brief Creates and registers material descriptor resources.
         *
         * Allocates a descriptor pool and descriptor sets, writes the
         * supplied material resource bindings, and transfers ownership
         * of the completed descriptor resources to the registry.
         *
         * Does not change the currently active descriptor handle.
         *
         * @param bindings Material resource bindings to write.
         *
         * @return Handle identifying the registered descriptor resources.
         *
         * @pre Descriptor-set layouts and pool requirements must be
         * initialized.
         * @pre Material bindings must be compatible with the layouts and
         * reference valid registered GPU resources.
         *
         * @post On success, the returned handle resolves to initialized
         * descriptor resources in m_MaterialDescriptorRegistry.
         * @post On failure, the active descriptor resources remain unchanged.
         * @post On success, dependencies on referenced textures and buffers
         * are registered in m_ResourceDependencies.
         * @post If an exception occurs, no newly registered descriptor
         * resources or dependency relationships remain.
         *
         * @throws Core::Error::EngineError if Vulkan descriptor allocation
         * fails.
         * @throws std::bad_alloc if host allocation fails.
         *
         * @note The caller must remove newly registered resources if a
         * subsequent step in its replacement transaction fails.
         * GPU resources referenced by the descriptors must remain alive.
         */
        [[nodiscard]] auto registerMaterialDescriptorResources(
            std::span<const MaterialResourceBinding> bindings)
            -> GpuResourceHandle;

        /**
         * @brief Creates and registers material descriptor resources for each
         * frame slot.
         *
         * Builds frame-specific material bindings and allocates descriptor
         * resources for every frame-in-flight slot.
         *
         * Each frame's descriptor sets reference its corresponding frequently
         * updated GPU buffers while preserving shared material resources, such
         * as textures and occasionally updated buffers.
         *
         * Descriptor resources are owned by m_MaterialDescriptorRegistry.
         * The returned handles reference those resources without taking
         * ownership.
         *
         * @param base_bindings Material bindings used to initialize each
         * frame's descriptor resources. Frequent-buffer handles are substituted
         * with the corresponding frame-specific handles.
         *
         * @return Registered material descriptor handles ordered by frame slot.
         *
         * @pre Frame resources must already be initialized.
         * @pre Descriptor-set layouts and pool requirements must be
         * initialized.
         * @pre All material bindings must reference valid, compatible GPU
         * resources.
         * @pre Frequently updated material buffers must have initialized GPU
         * buffers for every frame slot.
         *
         * @post On success, the returned vector contains one registered
         * descriptor handle per frame-in-flight slot.
         * @post Each descriptor resource references the appropriate
         * frame-specific and shared GPU resources.
         * @post Resource dependency relationships are registered for each
         * descriptor.
         * @post On failure, all descriptor resources registered during this
         * operation and their dependency relationships are rolled back.
         * @post The active material descriptor selection remains unchanged.
         *
         * @throws Core::Error::EngineError if Vulkan descriptor allocation
         * or initialization fails.
         * @throws std::bad_alloc if CPU storage, descriptor registration,
         * or dependency tracking allocation fails.
         *
         * @note This function does not select active descriptor resources,
         * modify m_FrameMaterialDescriptorHandles, or submit GPU work.
         *
         * @note Registered descriptors must remain alive while outstanding
         * GPU submissions reference them. Rollback is safe because newly
         * registered descriptors have not been submitted to the GPU.
         */
        [[nodiscard]] auto registerFrameMaterialDescriptorResources(
            std::span<const MaterialResourceBinding> base_bindings)
            -> std::vector<GpuResourceHandle>;

        /**
         * @brief Creates the registered descriptor state of a material
         * instance.
         *
         * Copies the material instance description and registers descriptor
         * resources compatible with the active material configuration.
         *
         * Uses shared descriptor resources when no frequently updated
         * material buffers exist, or frame-specific descriptor resources
         * otherwise.
         *
         * @param description Validated material instance description.
         * @return Prepared material instance resources containing the
         * copied description and registered descriptor handles.
         *
         * @pre The description must have passed material instance validation.
         * @pre Descriptor layouts and resource registries must be initialized.
         *
         * @post On success, descriptor resources are registered and tracked
         * through their resource dependencies.
         * @post On failure, no newly registered descriptor resources remain.
         * @post The logical material instance registry remains unchanged.
         *
         * @throws Core::Error::EngineError if Vulkan descriptor allocation
         * fails.
         * @throws std::bad_alloc if host allocation fails.
         *
         * @note The caller is responsible for rolling back the returned
         * descriptor handles if logical instance registration subsequently
         * fails.
         */
        [[nodiscard]] auto createMaterialInstanceResources(
            const MaterialInstanceDescription &description)
            -> MaterialInstanceResources;

        /**
         * @brief Rolls back a newly registered material descriptor resource.
         *
         * Removes tracked relationships involving the descriptor handle,
         * then removes the descriptor resources from their registry.
         *
         * This operation is intended only for resources that have not
         * been selected for rendering or referenced by GPU submissions.
         *
         * @param descriptor_handle Handle of the newly registered
         * descriptor resources to discard.
         *
         * @pre The handle must identify registered material descriptor
         * resources.
         * @pre The descriptor resources must not be active or in GPU use.
         *
         * @post The descriptor handle no longer resolves in the registry.
         * @post No tracked relationships reference the descriptor handle.
         *
         * @note This operation does not wait for GPU completion and must
         * not replace deferred retirement of previously active resources.
         */
        auto rollbackRegisteredMaterialDescriptorResources(
            GpuResourceHandle descriptor_handle) -> void;

        /**
         * @brief Rolls back multiple newly registered material descriptor
         * resources.
         *
         * Removes the descriptor resources identified by the supplied handles,
         * including their tracked resource dependency relationships.
         *
         * @param descriptor_handles Handles identifying descriptor resources
         * to discard.
         *
         * @pre Every handle must identify registered descriptor resources.
         * @pre None of the descriptor resources may be active or referenced
         * by outstanding GPU submissions.
         *
         * @post The supplied descriptor handles no longer resolve in the
         * registry, and their dependency relationships have been removed.
         *
         * @note This operation does not wait for GPU completion.
         * It must not be used to retire previously active descriptors.
         */
        auto rollbackRegisteredFrameMaterialDescriptorResources(
            std::span<const GpuResourceHandle> descriptor_handles) -> void;

        /**
         * @brief Collects the material descriptor handles currently used for
         * rendering.
         *
         * Returns frame-specific descriptor handles when configured, otherwise
         * returns the single shared material descriptor handle.
         *
         * Validates that frame-specific descriptors match the number of frame
         * resources and that every collected handle resolves to a registered
         * material descriptor resource.
         *
         * @return Handles identifying the current material descriptor
         * resources.
         *
         * @pre The renderer must have initialized material descriptor
         * resources.
         *
         * @post The renderer's resource ownership and active selections are
         * unchanged.
         *
         * @throws std::bad_alloc if allocating the returned handle collection
         * fails.
         */
        [[nodiscard]] auto collectActiveMaterialDescriptorHandles() const
            -> std::vector<GpuResourceHandle>;

        /**
         * @brief Validates an occasional material-buffer replacement.
         *
         * Finds the requested material binding and verifies that it references
         * a registered uniform or storage buffer.
         *
         * The replacement data must be nonempty and match the existing
         * buffer's size.
         *
         * @param set Descriptor-set index.
         * @param binding Binding index within the descriptor set.
         * @param array_element Descriptor array-element index.
         * @param data Replacement buffer contents.
         *
         * @return Index of the matching entry in m_MaterialBindings.bindings.
         *
         * @pre The requested binding must exist and reference a valid buffer.
         * @pre Replacement data must be nonempty and match the buffer size.
         *
         * @post Material bindings and GPU resources remain unchanged.
         *
         * @note Invalid input triggers a precondition assertion.
         * Invalid registered resource state triggers an invariant assertion.
         */
        [[nodiscard]] auto validateMaterialBufferReplacement(
            std::uint32_t set, std::uint32_t binding,
            std::uint32_t array_element, std::span<const std::byte> data) const
            -> std::size_t;

        /**
         * @brief Retires previous material resources after buffer replacement.
         *
         * Resolves the previous material descriptor handles and transfers their
         * resources to the graphics retirement queue.
         *
         * When requested, also retires the previous material buffer in the
         * same retirement record, ensuring dependent descriptor resources are
         * destroyed before the buffer.
         *
         * All resources remain alive until their final GPU submission
         * completes.
         *
         * @param old_buffer_handle Handle identifying the previous material
         * buffer.
         * @param previous_descriptor_handles Handles identifying the previous
         * material descriptor resources.
         * @param retire_old_buffer Whether to retire the previous buffer along
         * with the descriptor resources.
         *
         * @pre All supplied handles must resolve to registered resources.
         * @pre Previous descriptor handles must identify distinct resources.
         * @pre The graphics submission state must track all outstanding GPU
         * uses.
         *
         * @post On success, ownership of the selected resources is transferred
         * to the retirement queue.
         * @post If preparation fails, the original resources remain unchanged.
         *
         * @throws std::bad_alloc if retirement preparation fails.
         *
         * @note Does not remove registry entries or dependency relationships.
         * @note Does not wait for GPU completion.
         */
        auto retirePreviousMaterialResources(
            GpuResourceHandle old_buffer_handle,
            std::span<const GpuResourceHandle> previous_descriptor_handles,
            bool retire_old_buffer) -> void;

        /**
         * @brief Validates initial material bindings against shader
         * requirements.
         *
         * Checks that the registered material bindings satisfy the reflected
         * vertex and fragment shader descriptor requirements.
         *
         * @param interfaces Reflected interfaces of the initialized shaders.
         *
         * @pre The initial material resources must already be registered.
         *
         * @note Incompatible bindings trigger precondition assertions.
         */
        auto validateInitialMaterialBindings(
            const ReflectedGraphicsShaderInterfaces &interfaces) const -> void;

        /**
         * @brief Initializes the renderer's initial sampled texture.
         *
         * Creates the existing texture descriptor-set layout, uploads and
         * registers the texture, and establishes its material binding.
         *
         * The supplied CPU pixel storage is borrowed only during
         * initialization.
         *
         * @param texture Initial texture data and pixel storage.
         *
         * @post On success, the renderer owns the registered texture and
         * its initial descriptor binding.
         *
         * @throws Core::Error::EngineError if Vulkan resource creation or
         * upload fails.
         * @throws std::bad_alloc if resource storage allocation fails.
         *
         * @note Invalid texture data triggers precondition assertions.
         */
        auto initializeInitialTexture(const TextureUploadData &texture) -> void;

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
         * @param initial_buffer_bindings Optional material buffer bindings
         * initialized during renderer construction. The supplied data is
         * borrowed only until construction completes.
         * @param initial_texture_bindings Initial sampled-texture bindings.
         * Each input specifies a shader descriptor location and borrowed
         * texture upload data. Pixel storage must remain valid until
         * renderer construction completes.
         * @param require_swapchain_transfer_source Whether swapchain images
         * must support transfer-source operations for framebuffer readback. The
         * requirement is preserved across swapchain recreation. Defaults to
         * false.
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
                std::nullopt,
            std::span<const MaterialBufferBindingInput>
                initial_buffer_bindings = {},
            std::span<const MaterialTextureBindingInput>
                initial_texture_bindings = {},
            bool require_swapchain_transfer_source = false);

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
         * @brief Uploads and registers an independent GPU mesh.
         *
         * Creates vertex and index buffers and registers them with the
         * renderer's mesh resource registry.
         *
         * Registration does not replace the active mesh or modify draw
         * instances. The registered mesh remains owned by the renderer.
         *
         * @param mesh Borrowed vertex and index data to upload.
         * @return Handle identifying the registered GPU mesh.
         *
         * @pre A compatible vertex layout must already be configured.
         * @pre The mesh vertex stride must match the configured vertex stride.
         * @pre Registration must be serialized with rendering and other
         * resource operations.
         *
         * @post On success, the returned handle resolves to the registered
         * mesh.
         * @post The active mesh selection and draw instances remain unchanged.
         * @post On failure, no new mesh remains registered.
         * @post On success, vertex and index uploads have completed before
         * the mesh handle is returned.
         *
         * @note The current upload path uses synchronous immediate submission
         * on the graphics queue. Future asynchronous uploads may require
         * separate readiness tracking before registered resources can be used.
         *
         * @throws Core::Error::EngineError if GPU allocation or upload fails.
         * @throws std::bad_alloc if resource allocation or registration fails.
         *
         * @note Invalid mesh descriptions trigger precondition assertions.
         */
        [[nodiscard]] auto registerMesh(const MeshUploadData &mesh)
            -> GpuResourceHandle;

        /**
         * @brief Uploads and registers an independent GPU texture.
         *
         * Creates a sampled GPU image, image view, and sampler from the
         * supplied texture description.
         *
         * The resulting texture is owned by the renderer's texture registry
         * and identified through a generation-checked GPU resource handle.
         *
         * Registration does not create material descriptors, replace the
         * active texture, or modify existing material bindings.
         *
         * @param texture Borrowed CPU texture data and sampler configuration.
         * @return Handle identifying the registered GPU texture.
         *
         * @pre Texture dimensions, mip levels, pixel data, format, and
         * sampler configuration must satisfy the texture upload requirements.
         * @pre Registration must be serialized with rendering and other
         * resource operations.
         *
         * @post On success, the returned handle resolves to the registered
         * texture resources.
         * @post Existing textures and material bindings remain unchanged.
         * @post On failure, no new texture remains registered.
         * @post Texture upload and layout transitions complete before the
         * resource handle is returned.
         *
         * @throws Core::Error::EngineError if Vulkan resource creation
         * or texture upload fails.
         * @throws std::bad_alloc if resource allocation or registration fails.
         *
         * @note Invalid texture descriptions trigger precondition assertions.
         * @note Registered textures must remain alive while descriptors or
         * outstanding GPU submissions reference them.
         */
        [[nodiscard]] auto registerTexture(const TextureUploadData &texture)
            -> GpuResourceHandle;

        /**
         * @brief Creates and registers an independent material GPU buffer.
         *
         * Allocates and initializes a uniform or storage buffer using the
         * supplied CPU data.
         *
         * The buffer is owned by the renderer's buffer registry and is
         * identified through a generation-checked GPU resource handle.
         *
         * Registration does not create descriptor sets, modify material
         * bindings, or configure frame-specific buffer updates.
         *
         * @param data Borrowed initial buffer contents.
         * @param type Shader resource type determining the buffer usage.
         * @return Handle identifying the registered GPU buffer.
         *
         * @pre data must not be empty.
         * @pre type must be UniformBuffer or StorageBuffer.
         * @pre Registration must be serialized with rendering and other
         * resource operations.
         *
         * @post On success, the returned handle resolves to an initialized
         * buffer owned by the renderer.
         * @post Existing buffers and material bindings remain unchanged.
         * @post On failure, no new buffer remains registered.
         *
         * @throws Core::Error::EngineError if GPU buffer allocation or
         * initialization fails.
         * @throws std::bad_alloc if resource registration fails.
         *
         * @note The input data only needs to remain valid until registration
         * completes.
         * @note Registered buffers must remain alive while material
         * descriptors or outstanding GPU submissions reference them.
         */
        [[nodiscard]] auto
        registerMaterialBuffer(std::span<const std::byte> data,
                               Shader::ShaderResourceType type)
            -> GpuResourceHandle;

        /**
         * @brief Registers a material instance for rendering.
         *
         * Validates the supplied resource bindings against the active shader
         * interface and creates the required Vulkan descriptor resources.
         *
         * Material instance state is owned by m_MaterialInstanceRegistry.
         * Descriptor allocations remain owned by m_MaterialDescriptorRegistry.
         * Referenced textures and buffers retain their existing ownership.
         *
         * Registration does not change the default material or existing draw
         * instances.
         *
         * @param description Material instance configuration to register.
         * @return Handle identifying the registered material instance.
         *
         * @pre An active material configuration must be initialized.
         * @pre Resource bindings must satisfy the active shader interface.
         * @pre Referenced texture and buffer handles must resolve to registered
         * resources of the expected types.
         * @pre Frequently updated material buffers must use the renderer's
         * shared frame-specific buffer configuration.
         * @pre Registration must be serialized with rendering and other
         * resource operations.
         *
         * @post On success, the returned handle resolves to a registered
         * material instance.
         * @post Descriptor resources are registered for the required frame
         * slots when the material uses descriptors.
         * @post The default material and draw instances remain unchanged.
         * @post On failure, no new material instance, descriptor resources, or
         * dependency relationships remain registered.
         *
         * @throws Core::Error::EngineError if Vulkan descriptor resource
         * creation or allocation fails.
         * @throws std::bad_alloc if host resource allocation fails.
         *
         * @note Invalid material configurations and resource bindings trigger
         * precondition assertions.
         * @note Registration does not submit GPU rendering commands.
         */
        [[nodiscard]] auto
        registerMaterialInstance(const MaterialInstanceDescription &description)
            -> GpuResourceHandle;

        /**
         * @brief Unregisters an independent material instance.
         *
         * Removes a registered material instance and retires its descriptor
         * resources after all relevant GPU submissions have completed.
         *
         * The textures and buffers referenced by the material remain owned
         * by their respective resource registries.
         *
         * @param handle Handle identifying the material instance to remove.
         *
         * @pre The handle must identify a registered material instance.
         * @pre No active draw instance may reference the material.
         * @pre Removal must be serialized with rendering and other resource
         * operations.
         *
         * @post On success, the material handle becomes stale.
         * @post Descriptor resources remain alive until outstanding GPU
         * submissions referencing them have completed.
         * @post Descriptor dependency relationships are removed.
         * @post Referenced textures and buffers remain registered.
         * @post On failure, the material instance and descriptor resources
         * remain unchanged.
         *
         * @throws std::bad_alloc if retirement preparation fails.
         *
         * @note Invalid, stale, foreign, or referenced material handles
         * trigger precondition assertions.
         */
        auto unregisterMaterialInstance(GpuResourceHandle handle) -> void;

        /**
         * @brief Unregisters an independent GPU mesh.
         *
         * Removes a registered mesh that is no longer selected or referenced
         * by draw instances.
         *
         * The mesh's GPU buffers remain retained until all relevant graphics
         * submissions have completed.
         *
         * @param handle Handle identifying the mesh to unregister.
         *
         * @pre The handle must resolve to a mesh registered with this renderer.
         * @pre The mesh must not be the currently active mesh.
         * @pre No draw instances may reference the mesh.
         * @pre Removal must be serialized with rendering and other resource
         * operations.
         *
         * @post On success, the handle becomes stale and no longer resolves
         * through the mesh registry.
         * @post Previously submitted GPU work retains access to the retired
         * resources until completion.
         * @post On failure, the mesh remains registered and unchanged.
         *
         * @throws std::bad_alloc if retirement preparation fails.
         *
         * @note Invalid, stale, foreign, active, or referenced mesh handles
         * trigger precondition assertions.
         */
        auto unregisterMesh(GpuResourceHandle handle) -> void;

        /**
         * @brief Replaces the renderer's current mesh draw instances.
         *
         * Copies the supplied instances into renderer-owned storage.
         * Each instance references an existing registered GPU mesh through
         * a non-owning resource handle.
         *
         * The supplied instances remain active across frames until replaced.
         * An empty collection clears all draw instances.
         *
         * @param instances Borrowed mesh draw instances to configure.
         *
         * @pre Every mesh handle must resolve to a resource registered with
         * this renderer.
         * @pre The caller must serialize this operation with rendering and
         * other draw-instance modifications.
         *
         * @post On success, the renderer retains copies of the supplied
         * instances.
         * @post GPU mesh ownership and active mesh selection remain unchanged.
         * @post On failure, the previous draw instances remain unchanged.
         *
         * @throws std::bad_alloc if instance storage allocation fails.
         *
         * @note Invalid or stale mesh handles trigger precondition assertions.
         * @note The caller's storage only needs to remain valid until this
         * operation completes.
         */
        auto setDrawInstances(std::span<const MeshDrawInstance> instances)
            -> void;

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
         * @brief Replaces the active material's fragment shader.
         *
         * Validates and reflects the replacement shader, checks its
         * resource requirements against the existing material
         * configuration, and creates a replacement graphics pipeline
         * before modifying the active material.
         *
         * The operation does not recreate descriptor-set layouts or
         * material descriptor resources.
         *
         * @param fragment_shader Compiled fragment shader and selected
         * entry point.
         *
         * @pre An active material, mesh, and graphics pipeline must exist.
         * @pre The replacement must specify a valid fragment-stage entry
         * point and nonempty SPIR-V bytecode.
         * @pre The replacement's resource requirements must be compatible
         * with the existing descriptor-set layouts and material resources.
         * @pre The caller must serialize replacement with rendering and
         * other resource replacement operations.
         *
         * @post On success, subsequent frames use the replacement
         * fragment shader.
         *
         * @throws Core::Error::EngineError if shader validation, reflection,
         * resource compatibility checks, or pipeline creation fails.
         * @throws std::bad_alloc if host resource allocation fails.
         *
         * @note If preparation fails, the previously active shader and
         * graphics pipeline remain unchanged.
         * @note The previous pipeline must remain alive until all GPU
         * submissions referencing it have completed.
         */
        auto replaceFragmentShader(Shader::ShaderStageInput fragment_shader)
            -> void;

        /**
         * @brief Updates an existing material uniform or storage buffer.
         *
         * Creates a replacement buffer of the same resource type and size,
         * initializes it with the supplied data, and updates the material
         * descriptor resources to reference it.
         *
         * The previous buffer and descriptor resources remain alive until
         * graphics submissions referencing them have completed.
         *
         * The operation does not recreate the graphics pipeline.
         *
         * @param set Descriptor-set index containing the buffer.
         * @param binding Binding index within the descriptor set.
         * @param array_element Element index within the descriptor array.
         * @param data Borrowed CPU data used to initialize the replacement
         * buffer.
         *
         * @pre Material descriptor resources must exist.
         * @pre The specified binding must reference an existing uniform or
         * storage buffer.
         * @pre Data must not be empty and must match the existing buffer size.
         * @pre The caller must serialize this operation with rendering and
         * other resource replacement operations.
         *
         * @post On success, subsequent frames use the updated buffer data.
         * @post If preparation or retirement fails, the existing material
         * remains active.
         *
         * @throws Core::Error::EngineError if buffer or descriptor resource
         * creation fails.
         * @throws std::bad_alloc if resource allocation fails.
         *
         * @note The supplied CPU data must remain valid until this operation
         * completes.
         */
        auto updateMaterialBuffer(std::uint32_t set, std::uint32_t binding,
                                  std::uint32_t array_element,
                                  std::span<const std::byte> data) -> void;

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
         * @param record_external_commands Optional callback invoked during
         * active dynamic rendering, after mesh drawing and before rendering
         * ends. An empty callback records no additional commands.
         *
         * @pre The callback must not end the rendering scope, submit or reset
         * the command buffer, or retain it after invocation.
         * @pre The callback must not throw exceptions.
         *
         * @throws Core::Error::EngineError if a required Vulkan frame operation
         * fails.
         *
         * @note A failed Vulkan queue submission permanently prevents
         * further frame rendering through this renderer instance.
         * Subsequent renderFrame() calls throw rather than waiting
         * on potentially unsignaled synchronization objects.
         */
        auto renderFrame(
            const VulkanFrameCommandRecorder &record_external_commands = {})
            -> void;

        /**
         * @brief Updates the model transformation of an existing draw instance.
         *
         * Replaces the model matrix of the specified renderer-managed
         * draw instance without modifying its mesh resource handle.
         *
         * The updated transformation is used when recording
         * subsequent graphics commands.
         *
         * @param instance_index Zero-based index of the draw instance.
         * @param model_matrix Replacement model transformation.
         *
         * @pre instance_index must identify an existing draw instance.
         * @pre Updates must be serialized with frame command recording.
         *
         * @post The selected instance uses the supplied model matrix.
         * @post All other draw instances remain unchanged.
         *
         * @note This operation does not upload mesh data or recreate
         * GPU resources.
         */
        auto
        setDrawInstanceModelMatrix(std::size_t instance_index,
                                   const Core::Math::Matrix4f &model_matrix)
            -> void;

        /**
         * @brief Updates the camera's world-to-view transformation.
         *
         * Replaces the view matrix used when recording subsequent frames.
         * Preserves the existing camera projection parameters.
         *
         * The supplied matrix represents the transformation from world
         * coordinates into camera coordinates, not the camera's world-space
         * model transformation.
         *
         * @param view_matrix Replacement world-to-view matrix.
         *
         * @pre An initial camera configuration must exist.
         * @pre Every matrix element must be finite.
         * @pre Updates must be serialized with frame command recording.
         *
         * @post Subsequent frames use the updated camera view matrix.
         *
         * @note Does not recreate graphics pipelines, upload GPU resources,
         * or modify swapchain resources.
         */
        auto setCameraViewMatrix(const Core::Math::Matrix4f &view_matrix)
            -> void;

        /**
         * @brief Returns Vulkan handles and configuration for external
         * rendering.
         *
         * Provides a snapshot of the renderer's Vulkan initialization state
         * and current swapchain color format and image count.
         *
         * All returned Vulkan handles are borrowed. The caller must not destroy
         * them or use them after their owning renderer resources are destroyed.
         *
         * Swapchain-dependent values must be refreshed following swapchain
         * recreation.
         *
         * @return Vulkan handles and current rendering configuration.
         *
         * @pre Swapchain resources must be initialized.
         *
         * @note The returned information does not transfer resource ownership.
         */
        [[nodiscard]] auto externalRenderingInfo() const
            -> VulkanExternalRenderingInfo;
    };
} // namespace SNE::Engine::Renderer::Vulkan
