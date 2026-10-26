#pragma once

#include "engine/renderer/vulkan/vulkan_external_rendering_info.hpp"

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace SNE::Editor {
    /**
     * @brief Owns the Dear ImGui Vulkan rendering backend.
     *
     * Initializes Dear ImGui's Vulkan backend using borrowed Vulkan handles
     * and swapchain configuration supplied by the engine renderer.
     *
     * The backend manages its own ImGui rendering resources but does not
     * own the Vulkan instance, physical device, logical device, graphics
     * queue, or swapchain.
     *
     * An initialized Dear ImGui context must exist before construction
     * and remain alive until after this object is destroyed.
     *
     * The engine renderer and its Vulkan device must also outlive this
     * object.
     *
     * This class is non-copyable and non-movable because it manages the
     * lifetime of a backend associated with the active ImGui context.
     */
    class EditorImGuiVulkanBackend {
      private:
        /**
         * @brief Borrowed logical-device handle used during backend shutdown.
         *
         * Remains owned by the engine renderer and must outlive this object.
         * VK_NULL_HANDLE indicates that initialization has not completed.
         */
        VkDevice m_Device{VK_NULL_HANDLE};

      public:
        /**
         * @brief Initializes the Dear ImGui Vulkan rendering backend.
         *
         * Configures ImGui to render using the engine's existing Vulkan device,
         * graphics queue, and swapchain attachment format.
         *
         * @param rendering_info Borrowed Vulkan handles and current rendering
         * configuration supplied by the engine renderer.
         *
         * @pre A valid Dear ImGui context must be active.
         * @pre All required Vulkan handles must be valid.
         * @pre The swapchain must have a valid color attachment format and
         * at least two images.
         *
         * @throws SNE::Engine::Core::Error::EngineError if backend
         * initialization fails.
         */
        explicit EditorImGuiVulkanBackend(
            const Vulkan::VulkanExternalRenderingInfo &rendering_info);

        /**
         * @brief Shuts down the Dear ImGui Vulkan rendering backend.
         *
         * Ensures outstanding GPU operations have completed before releasing
         * backend-owned Vulkan resources.
         *
         * Does not destroy the borrowed logical device or other engine-owned
         * Vulkan resources.
         */
        ~EditorImGuiVulkanBackend();

        EditorImGuiVulkanBackend(const EditorImGuiVulkanBackend &) = delete;
        auto operator=(const EditorImGuiVulkanBackend &)
            -> EditorImGuiVulkanBackend & = delete;

        EditorImGuiVulkanBackend(EditorImGuiVulkanBackend &&) = delete;
        auto operator=(EditorImGuiVulkanBackend &&)
            -> EditorImGuiVulkanBackend & = delete;
    };
} // namespace SNE::Editor
