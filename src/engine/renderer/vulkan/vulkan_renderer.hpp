#pragma once

#include "engine/platform/window.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "vulkan_device.hpp"
#include "vulkan_device_features.hpp"
#include "vulkan_device_selection.hpp"
#include "vulkan_frame_resources.hpp"
#include "vulkan_graphics_pipeline.hpp"
#include "vulkan_instance.hpp"
#include "vulkan_memory_allocator.hpp"
#include "vulkan_semaphore.hpp"
#include "vulkan_surface.hpp"
#include "vulkan_swapchain.hpp"
#include <cstddef>
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
     * The type is non-copyable because it owns Vulkan resources with exclusive
     * lifetimes.
     */
    class VulkanRenderer {
      private:
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
         * @brief Presentation swapchain owned by the renderer.
         *
         * Owns the swapchain and its image views while exposing the
         * swapchain-provided images as non-owning handles.
         *
         * Swapchain-dependent resources must be recreated when this swapchain
         * is replaced.
         */
        VulkanSwapchain m_Swapchain;
        /**
         * @brief Graphics pipeline owned by the renderer.
         *
         * Owns the Vulkan graphics pipeline and pipeline layout used for
         * graphics command recording.
         *
         * The pipeline is created for the swapchain color-attachment format and
         * must be recreated if pipeline compatibility requirements change.
         *
         * The logical device and compatible rendering configuration must
         * outlive this object.
         */
        VulkanGraphicsPipeline m_GraphicsPipeline;
        /**
         * @brief Presentation-wait semaphores associated with swapchain images.
         *
         * Stores one semaphore for each swapchain image. The semaphore selected
         * by an acquired image index is signaled when rendering for that image
         * completes and is subsequently waited on by presentation.
         *
         * The collection follows swapchain-image lifetime and must be recreated
         * when the corresponding swapchain images are replaced.
         */
        std::vector<VulkanSemaphore> m_RenderFinishedSemaphores;
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

      public:
        /**
         * @brief Initializes the Vulkan renderer for an application window.
         *
         * Creates the Vulkan instance and presentation surface, selects a
         * suitable physical device, negotiates logical-device features, creates
         * the logical device and GPU memory allocator, creates the presentation
         * swapchain and graphics pipeline, and establishes the initial command
         * infrastructure.
         *
         * @param application_name Name reported to Vulkan for the application.
         * @param window Platform window used for Vulkan surface creation and
         * framebuffer sizing.
         * @param presentation_preference Presentation behavior requested by the
         * renderer.
         * @param development_diagnostics_enabled Whether development-oriented
         * Vulkan diagnostics should be enabled.
         *
         * @throws Core::Error::EngineError if a required Vulkan resource or
         * capability cannot be initialized.
         */
        VulkanRenderer(const std::string &application_name,
                       const Platform::Window &window,
                       PresentationPreference presentation_preference,
                       bool development_diagnostics_enabled);

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
