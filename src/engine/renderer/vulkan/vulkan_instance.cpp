#include "vulkan_instance.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "vulkan_result.hpp"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <string>

namespace SNE::Engine::Renderer::Vulkan {
    VulkanInstance::VulkanInstance() {

        VkApplicationInfo application_info{};
        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.pApplicationName = "Signum Editor";
        application_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
        application_info.pEngineName = "Signum Engine";
        application_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
        application_info.apiVersion = VK_API_VERSION_1_4;

        std::uint32_t required_extension_count{};
        VkInstanceCreateInfo instance_create_info{};
        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo = &application_info;
        const char **required_extensions =
            glfwGetRequiredInstanceExtensions(&required_extension_count);
        if (required_extensions == nullptr) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanRequiredExtensionsUnavailable,
                "Failed to get required Vulkan instance extensions",
                "Get Required Vulkan Instance Extensions");
        }
        instance_create_info.ppEnabledExtensionNames = required_extensions;
        instance_create_info.enabledExtensionCount = required_extension_count;

        const VkResult result =
            vkCreateInstance(&instance_create_info, nullptr, &m_Instance);

        if (result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanInstanceCreationFailed,
                "Failed to create a Vulkan instance",
                Core::Error::NativeError(static_cast<int>(result),
                                         std::string(toString(result))),
                "Create Vulkan Instance");
        }
    }

    VulkanInstance::~VulkanInstance() {
        vkDestroyInstance(m_Instance, nullptr);
    }
} // namespace SNE::Engine::Renderer::Vulkan
