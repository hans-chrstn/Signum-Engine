#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

struct GLFWwindow;

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Owns a Vulkan presentation surface associated with a GLFW window.
     *
     * VulkanSurface creates and manages the lifetime of a VkSurfaceKHR used to
     * present rendered images to a native window.
     *
     * The Vulkan instance and GLFW window passed during construction are
     * borrowed dependencies and must remain valid for the lifetime of this
     * object.
     *
     * The type is non-copyable because it exclusively owns the Vulkan surface.
     */
    class VulkanSurface {
      private:
        VkInstance m_Instance{VK_NULL_HANDLE};
        VkSurfaceKHR m_Surface{VK_NULL_HANDLE};

      public:
        /**
         * @brief Creates a Vulkan surface for a GLFW window.
         *
         * @param instance Vulkan instance used to create and later destroy the
         * surface.
         * @param window GLFW window associated with the surface.
         *
         * @throws Core::Error::EngineError if the Vulkan surface cannot be
         * created.
         */
        VulkanSurface(VkInstance instance, GLFWwindow *window);

        /**
         * @brief Destroys the owned Vulkan surface.
         */
        ~VulkanSurface();
        VulkanSurface(const VulkanSurface &) = delete;
        auto operator=(const VulkanSurface &) -> VulkanSurface & = delete;

        /**
         * @brief Returns the underlying Vulkan surface handle.
         *
         * The returned handle is non-owning and remains valid only while this
         * VulkanSurface object remains alive.
         *
         * @return Vulkan surface handle owned by this object.
         */
        [[nodiscard]] auto nativeHandle() const -> VkSurfaceKHR;
    };
} // namespace SNE::Engine::Renderer::Vulkan
