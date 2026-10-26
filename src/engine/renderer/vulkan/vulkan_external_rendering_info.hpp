#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Describes Vulkan resources available to external rendering
     * systems.
     *
     * Provides borrowed Vulkan handles and rendering configuration needed
     * to initialize an external graphics backend.
     *
     * VulkanRenderer retains ownership of the Vulkan instance, physical
     * device selection, logical device, queues, and swapchain resources.
     *
     * The stored handles must not be destroyed by consumers and must not
     * be used after their owning renderer resources are destroyed.
     *
     * Swapchain-dependent information may change when the swapchain is
     * recreated. Consumers must refresh that information and update
     * affected resources before further rendering.
     *
     * This type does not depend on Dear ImGui or own Vulkan resources.
     */
    struct VulkanExternalRenderingInfo {
        /** Borrowed Vulkan instance handle owned by the renderer. */
        VkInstance instance{VK_NULL_HANDLE};

        /** Borrowed physical-device handle selected by the renderer. */
        VkPhysicalDevice physical_device{VK_NULL_HANDLE};

        /** Borrowed logical-device handle owned by the renderer. */
        VkDevice device{VK_NULL_HANDLE};

        /** Borrowed graphics queue used for rendering submissions. */
        VkQueue graphics_queue{VK_NULL_HANDLE};

        /** Queue-family index associated with the graphics queue. */
        std::uint32_t graphics_queue_family_index{};

        /** Vulkan API version used to initialize the renderer. */
        std::uint32_t api_version{};

        /** Color attachment format selected for the current swapchain. */
        VkFormat color_attachment_format{VK_FORMAT_UNDEFINED};

        /**
         * @brief Depth attachment format used by the renderer.
         *
         * Must match the depth attachment format used by the active
         * dynamic rendering scope.
         *
         * May change when swapchain-dependent resources are recreated.
         */
        VkFormat depth_attachment_format{VK_FORMAT_UNDEFINED};

        /** Number of images owned by the current swapchain. */
        std::uint32_t swapchain_image_count{};
    };
} // namespace SNE::Engine::Renderer::Vulkan
