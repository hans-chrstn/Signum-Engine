#pragma once

#include "engine/core/math/matrix4.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/geometry/initial_mesh_data.hpp"
#include "engine/renderer/geometry/mesh_upload_data.hpp"
#include "engine/renderer/geometry/vertex_attribute_description.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/vulkan/command/vulkan_immediate_submission.hpp"
#include "engine/renderer/vulkan/device/vulkan_device.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_features.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_selection.hpp"
#include "engine/renderer/vulkan/memory/vulkan_buffer.hpp"
#include "engine/renderer/vulkan/memory/vulkan_memory_allocator.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_graphics_pipeline.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_surface.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_swapchain.hpp"
#include "engine/renderer/vulkan/synchronization/vulkan_semaphore.hpp"
#include "vulkan_frame_resources.hpp"
#include "vulkan_instance.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

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
        struct SwapchainResources {
            VulkanSwapchain m_VulkanSwapchain;
            std::optional<VulkanGraphicsPipeline> m_VulkanGraphicsPipeline;
            std::vector<VulkanSemaphore> m_VulkanSemaphores;
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
         * @brief Owns the GPU vertex buffer for the uploaded mesh.
         *
         * Stores device-local vertex data supplied during renderer
         * initialization.
         *
         * The buffer is independent of swapchain resources and survives
         * swapchain recreation.
         *
         * The buffer must be released before the Vulkan memory allocator and
         * logical device are destroyed.
         */
        std::optional<VulkanBuffer> m_MeshVertexBuffer;

        /**
         * @brief Owns the GPU index buffer for the uploaded mesh.
         *
         * Stores device-local indices referencing vertices in the mesh's vertex
         * buffer.
         *
         * The buffer is independent of swapchain resources and survives
         * swapchain recreation.
         *
         * The buffer must be released before the Vulkan memory allocator and
         * logical device are destroyed.
         */
        std::optional<VulkanBuffer> m_MeshIndexBuffer;

        /**
         * @brief Number of indices in the currently uploaded GPU mesh.
         *
         * Updated after successful mesh upload and used to determine the index
         * count supplied to indexed draw commands.
         *
         * The count remains associated with the renderer-owned mesh buffers and
         * is independent of swapchain recreation.
         */
        std::uint32_t m_MeshIndexCount{};

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
         * @brief Swapchain-dependent renderer resources when presentation is
         * available.
         *
         * Owns the presentation swapchain, optional graphics pipeline, and
         * render-finished semaphores associated with swapchain images.
         *
         * The optional is empty while no drawable framebuffer is available,
         * allowing the renderer to remain initialized without presentation
         * resources.
         *
         * When present, the swapchain and render-finished semaphores are
         * initialized together. A graphics pipeline is only required when
         * geometry is available for drawing.
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
         * @brief Index of the frame-in-flight slot currently being processed.
         *
         * Selects the VulkanFrameResources object used for the current renderer
         * frame. The index advances after a frame is completed and wraps
         * through the available frame-resource slots.
         */
        std::size_t m_CurrentFrameIndex{};

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
         */
        [[nodiscard]] auto recreateSwapchainResources() -> bool;

        /**
         * @brief Creates and uploads GPU buffers for the supplied mesh.
         *
         * Consumes borrowed CPU-side vertex and index data to create
         * device-local Vulkan buffers through the renderer's memory allocator
         * and immediate submission infrastructure.
         *
         * The renderer retains ownership of the resulting GPU buffers. The
         * supplied mesh data is not retained, and its underlying CPU storage
         * must remain valid until the upload operation completes.
         *
         * Initially intended for renderer initialization before normal frame
         * submission begins. Runtime mesh replacement and synchronization with
         * in-flight rendering work are not supported by this contract.
         *
         * @param mesh Non-owning description of vertex data, vertex stride, and
         * index data to upload.
         *
         * @pre Vertex data and indices must be nonempty.
         * @pre Vertex stride must be nonzero and divide the vertex byte count
         * evenly.
         * @pre Every index must reference an existing vertex.
         * @pre The renderer's memory allocator and immediate submission
         * resources must be initialized.
         *
         * @post On successful completion, the renderer owns device-local vertex
         * and index buffers containing the supplied mesh data.
         *
         * @throws Core::Error::EngineError if Vulkan buffer creation, memory
         * allocation, or data upload fails.
         *
         * @note Invalid mesh descriptions trigger fatal precondition
         * assertions.
         */
        auto uploadMesh(const MeshUploadData &mesh) -> void;

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
         *
         * @throws Core::Error::EngineError if a required Vulkan resource or
         * capability cannot be initialized.
         */
        VulkanRenderer(
            const std::string &application_name, const Platform::Window &window,
            PresentationPreference presentation_preference,
            bool development_diagnostics_enabled,
            std::optional<InitialMeshData> initial_mesh = std::nullopt);

        /**
         * @brief Releases the Vulkan renderer runtime.
         *
         * Waits for outstanding Vulkan device work to complete before
         * renderer-owned frame, synchronization, graphics, swapchain,
         * memory-allocation, and device resources are destroyed in reverse
         * dependency order.
         *
         * Shutdown synchronization is best-effort because destruction must not
         * propagate exceptions.
         */
        ~VulkanRenderer() noexcept;

        VulkanRenderer(const VulkanRenderer &) = delete;

        auto operator=(const VulkanRenderer &) -> VulkanRenderer & = delete;

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
         */
        auto renderFrame() -> void;
    };
} // namespace SNE::Engine::Renderer::Vulkan
