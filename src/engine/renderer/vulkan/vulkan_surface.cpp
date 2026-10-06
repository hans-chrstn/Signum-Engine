#include "vulkan_surface.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include "vulkan_result.hpp"
#include <GLFW/glfw3.h>
#include <string>

namespace SNE::Engine::Renderer::Vulkan {
    VulkanSurface::VulkanSurface(VkInstance instance, GLFWwindow *window)
        : m_Instance(instance) {
        if (instance == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanSurface requires a valid Vulkan instance");
        }

        if (window == nullptr) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanSurface requires a valid GLFW window");
        }

        const VkResult result =
            glfwCreateWindowSurface(m_Instance, window, nullptr, &m_Surface);

        if (result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanSurfaceCreationFailed,
                "Failed to create a window surface",
                Core::Error::NativeError(static_cast<int>(result),
                                         std::string(toString(result))),
                "Create Vulkan Window Surface");
        }

        if (m_Surface == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Postcondition,
                Core::Error::Subsystem::Vulkan,
                "Successful Vulkan surface creation must produce a non-null "
                "surface handle");
        }
    }

    VulkanSurface::~VulkanSurface() {
        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    }

    auto VulkanSurface::nativeHandle() const -> VkSurfaceKHR {
        return m_Surface;
    }
} // namespace SNE::Engine::Renderer::Vulkan
