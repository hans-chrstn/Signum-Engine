#include "vulkan_instance.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "vulkan_result.hpp"
#include <GLFW/glfw3.h>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace SNE::Engine::Renderer::Vulkan {

    constexpr const char *kValidationLayerName = "VK_LAYER_KHRONOS_validation";
    constexpr std::uint32_t kRequiredApiVersion = VK_API_VERSION_1_4;

    // Instance creation succeeds before debug-messenger creation. If the latter
    // fails, release the already-created Vulkan instance before propagating the
    // exception so construction remains leak-free.
    VulkanInstance::VulkanInstance() {
        createInstance();

        try {
            setupDebugMessenger();
        } catch (...) {
            vkDestroyInstance(m_Instance, nullptr);
            m_Instance = VK_NULL_HANDLE;
            throw;
        }
    }

    auto VulkanInstance::createInstance() -> void {
        const bool layer_support = checkValidationLayerSupport();
        if (!layer_support) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanValidationLayerUnavailable,
                "Vulkan instance has no validation layer support",
                "Check Vulkan Validation Layer Support");
        }

        VkApplicationInfo application_info{};
        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.pApplicationName = "Signum Editor";
        application_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
        application_info.pEngineName = "Signum Engine";
        application_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);

        const std::uint32_t supported_api_version = querySupportedApiVersion();
        if (supported_api_version < kRequiredApiVersion) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanApiVersionUnsupported,
                "The Vulkan loader does not support Signum's required API "
                "version",
                "Validate Vulkan Instance API Version");
        }
        application_info.apiVersion = kRequiredApiVersion;

        const std::vector<const char *> extensions = getRequiredExtensions();
        const bool extension_support =
            checkRequiredExtensionSupport(extensions);
        if (!extension_support) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanExtensionSupportUnavailable,
                "One or more required Vulkan instance extensions are "
                "unavailable.",
                "Check Vulkan Extension Support");
        }

        const VkDebugUtilsMessengerCreateInfoEXT debug_create_info =
            makeDebugMessengerCreateInfo();

        VkInstanceCreateInfo instance_create_info{};
        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo = &application_info;
        instance_create_info.ppEnabledExtensionNames = extensions.data();
        instance_create_info.enabledExtensionCount =
            static_cast<std::uint32_t>(extensions.size());
        instance_create_info.ppEnabledLayerNames = &kValidationLayerName;
        instance_create_info.enabledLayerCount = 1;

        // Chain the debug-messenger configuration into instance creation so
        // validation messages can be captured during vkCreateInstance and
        // vkDestroyInstance as well as during the normal instance lifetime.
        instance_create_info.pNext = &debug_create_info;

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

    // Vulkan enumeration is a two-call operation, and the available layer count
    // may change between calls. Retry the complete enumeration when Vulkan
    // reports VK_INCOMPLETE.
    auto VulkanInstance::checkValidationLayerSupport() -> bool {
        std::vector<VkLayerProperties> available_layers;

        while (true) {
            std::uint32_t layer_count{};
            const VkResult count_result =
                vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

            if (count_result != VK_SUCCESS) {
                throw Core::Error::EngineError(
                    Core::Error::Code::VulkanLayerEnumerationFailed,
                    "Failed to enumerate Vulkan instance layers",
                    Core::Error::NativeError(
                        static_cast<int>(count_result),
                        std::string(toString(count_result))),
                    "Enumerate Vulkan Instance Layers");
            }

            available_layers.resize(layer_count);

            const VkResult enumeration_result =
                vkEnumerateInstanceLayerProperties(&layer_count,
                                                   available_layers.data());

            if (enumeration_result == VK_SUCCESS) {
                available_layers.resize(layer_count);
                break;
            }

            if (enumeration_result == VK_INCOMPLETE) {
                continue;
            }

            throw Core::Error::EngineError(
                Core::Error::Code::VulkanLayerEnumerationFailed,
                "Failed to enumerate Vulkan instance layers",
                Core::Error::NativeError(
                    static_cast<int>(enumeration_result),
                    std::string(toString(enumeration_result))),
                "Enumerate Vulkan Instance Layers");
        }

        for (const VkLayerProperties &layer : available_layers) {
            if (std::strcmp(layer.layerName, kValidationLayerName) == 0) {
                return true;
            }
        }

        return false;
    }

    auto VulkanInstance::getRequiredExtensions() -> std::vector<const char *> {
        std::uint32_t glfw_extension_count{};
        const char **glfw_extensions =
            glfwGetRequiredInstanceExtensions(&glfw_extension_count);

        if (glfw_extensions == nullptr) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanRequiredExtensionsUnavailable,
                "Failed to get required Vulkan instance extensions",
                "Get Required Vulkan Instance Extensions");
        }

        std::vector<const char *> required_extensions;

        for (std::uint32_t i{}; i < glfw_extension_count; ++i) {
            required_extensions.push_back(glfw_extensions[i]);
        }

        // The debug-utils extension is required for the validation debug
        // messenger created by this class.
        required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return required_extensions;
    }

    auto VulkanInstance::checkRequiredExtensionSupport(
        const std::vector<const char *> &required_extensions) -> bool {

        std::vector<VkExtensionProperties> available_extensions;
        while (true) {
            std::uint32_t extension_count{};
            const VkResult count_result =
                vkEnumerateInstanceExtensionProperties(
                    nullptr, &extension_count, nullptr);
            if (count_result != VK_SUCCESS) {
                throw Core::Error::EngineError(
                    Core::Error::Code::VulkanExtensionEnumerationFailed,
                    "Failed to enumerate Vulkan extension properties",
                    Core::Error::NativeError(
                        static_cast<int>(count_result),
                        std::string(toString(count_result))),
                    "Enumerate Vulkan Instance Extensions");
            }

            available_extensions.resize(extension_count);

            const VkResult extension_result =
                vkEnumerateInstanceExtensionProperties(
                    nullptr, &extension_count, available_extensions.data());

            if (extension_result == VK_SUCCESS) {
                available_extensions.resize(extension_count);
                break;
            }

            if (extension_result == VK_INCOMPLETE) {
                continue;
            }

            throw Core::Error::EngineError(
                Core::Error::Code::VulkanExtensionEnumerationFailed,
                "Failed to enumerate Vulkan extension properties",
                Core::Error::NativeError(
                    static_cast<int>(extension_result),
                    std::string(toString(extension_result))),
                "Enumerate Vulkan Instance Extensions");
        }

        for (const char *required_extension : required_extensions) {
            bool found = false;
            for (const VkExtensionProperties &extension :
                 available_extensions) {
                if (std::strcmp(required_extension, extension.extensionName) ==
                    0) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                return false;
            }
        }
        return true;
    }

    auto VulkanInstance::makeDebugMessengerCreateInfo()
        -> VkDebugUtilsMessengerCreateInfoEXT {
        VkDebugUtilsMessengerCreateInfoEXT create_info{};

        create_info.sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        create_info.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        create_info.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        create_info.pfnUserCallback = debugCallback;
        create_info.pUserData = nullptr;

        return create_info;
    }

    auto VulkanInstance::setupDebugMessenger() -> void {
        const VkDebugUtilsMessengerCreateInfoEXT create_info =
            makeDebugMessengerCreateInfo();

        const auto create_debug_messenger =
            reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                // VK_EXT_debug_utils commands are extension functions and must
                // be resolved from the created Vulkan instance before use.
                vkGetInstanceProcAddr(m_Instance,
                                      "vkCreateDebugUtilsMessengerEXT"));

        if (create_debug_messenger == nullptr) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanDebugMessengerFunctionUnavailable,
                "Vulkan debug messenger creation function is unavailable",
                "Get Vulkan Debug Messenger Function");
        }

        const VkResult result = create_debug_messenger(
            m_Instance, &create_info, nullptr, &m_DebugMessenger);

        if (result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanDebugMessengerCreationFailed,
                "Failed to create Vulkan debug messenger",
                Core::Error::NativeError(static_cast<int>(result),
                                         std::string(toString(result))),
                "Create Vulkan Debug Messenger");
        }
    }

    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInstance::debugCallback(
        [[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT
            message_severity,
        [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
        [[maybe_unused]] void *user_data) {

        std::cerr << "Vulkan validation: " << callback_data->pMessage << '\n';

        return VK_FALSE;
    }

    auto VulkanInstance::destroyDebugMessenger() noexcept -> void {
        if (m_DebugMessenger == VK_NULL_HANDLE) {
            return;
        }

        const auto destroy_debug_messenger =
            reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                // Resolve the extension destruction function from the owning
                // instance.
                vkGetInstanceProcAddr(m_Instance,
                                      "vkDestroyDebugUtilsMessengerEXT"));

        if (destroy_debug_messenger != nullptr) {
            destroy_debug_messenger(m_Instance, m_DebugMessenger, nullptr);
        }

        m_DebugMessenger = VK_NULL_HANDLE;
    }

    auto VulkanInstance::querySupportedApiVersion() -> std::uint32_t {
        std::uint32_t version{};
        const VkResult result = vkEnumerateInstanceVersion(&version);

        if (result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanApiVersionQueryFailed,
                "Failed to query the Vulkan loader's supported instance API "
                "version",
                Core::Error::NativeError(static_cast<int>(result),
                                         std::string(toString(result))),
                "Query Vulkan Instance API Version");
        }

        return version;
    }

    VulkanInstance::~VulkanInstance() {
        destroyDebugMessenger();
        vkDestroyInstance(m_Instance, nullptr);
    }
} // namespace SNE::Engine::Renderer::Vulkan
