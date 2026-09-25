#pragma once

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Owns the engine's Vulkan instance and instance-level debug
     * messenger.
     *
     * Construction validates the required instance configuration, creates the
     * Vulkan instance, and establishes the debug messenger used by the
     * validation system.
     *
     * The Vulkan instance uses the instance extensions required by GLFW
     * together with the Vulkan debug-utils extension.
     *
     * Destruction releases the debug messenger before destroying the Vulkan
     * instance.
     *
     * The type is non-copyable because it exclusively owns Vulkan handles.
     */
    class VulkanInstance {
      private:
        VkInstance m_Instance{VK_NULL_HANDLE};
        VkDebugUtilsMessengerEXT m_DebugMessenger{VK_NULL_HANDLE};
        auto createInstance() -> void;
        [[nodiscard]] static auto checkValidationLayerSupport() -> bool;
        [[nodiscard]] static auto getRequiredExtensions()
            -> std::vector<const char *>;
        auto setupDebugMessenger() -> void;
        static VKAPI_ATTR VkBool32 VKAPI_CALL
        // NOLINTNEXTLINE(modernize-use-trailing-return-type)
        debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                      VkDebugUtilsMessageTypeFlagsEXT message_type,
                      const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                      void *user_data);
        auto destroyDebugMessenger() noexcept -> void;
        [[nodiscard]] static auto makeDebugMessengerCreateInfo()
            -> VkDebugUtilsMessengerCreateInfoEXT;
        [[nodiscard]] static auto checkRequiredExtensionSupport(
            const std::vector<const char *> &required_extensions) -> bool;
        [[nodiscard]] static auto querySupportedApiVersion() -> std::uint32_t;

      public:
        /**
         * @brief Creates the engine's Vulkan instance and debug messenger.
         *
         * @throws Core::Error::EngineError if validation-layer discovery,
         * required-extension discovery or validation, API-version discovery or
         * validation, Vulkan instance creation, or debug-messenger creation
         * fails.
         */
        VulkanInstance();

        /**
         * @brief Releases the Vulkan resources owned by this object.
         *
         * The debug messenger is destroyed before the Vulkan instance.
         */
        ~VulkanInstance();

        VulkanInstance(const VulkanInstance &) = delete;
        auto operator=(const VulkanInstance &) -> VulkanInstance & = delete;

        /**
         * @brief Returns the underlying Vulkan instance handle.
         *
         * The returned handle is non-owning and remains valid only while this
         * VulkanInstance object remains alive.
         *
         * @return Vulkan instance handle owned by this object.
         */
        [[nodiscard]] auto nativeHandle() const -> VkInstance;
    };
} // namespace SNE::Engine::Renderer::Vulkan
