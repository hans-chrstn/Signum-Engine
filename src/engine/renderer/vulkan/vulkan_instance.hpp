#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Owns the engine's Vulkan instance and optional debug messenger.
     *
     * Construction validates the required instance configuration and creates
     * the Vulkan instance. When development diagnostics are enabled, Vulkan
     * validation, debug-utils support, and the instance-level debug messenger
     * are also enabled.
     *
     * The Vulkan instance always enables the instance extensions required by
     * the platform. Diagnostic extensions and layers are enabled only when
     * requested by application policy.
     *
     * Destruction releases the debug messenger when present before destroying
     * the Vulkan instance.
     *
     * The type is non-copyable because it exclusively owns Vulkan handles.
     */
    class VulkanInstance {
      private:
        VkInstance m_Instance{VK_NULL_HANDLE};
        VkDebugUtilsMessengerEXT m_DebugMessenger{VK_NULL_HANDLE};
        auto createInstance(const std::string &application_name,
                            bool development_diagnostics_enabled) -> void;
        [[nodiscard]] static auto checkValidationLayerSupport() -> bool;
        [[nodiscard]] static auto
        getRequiredExtensions(bool development_diagnostics_enabled)
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
            std::span<const char *const> required_extensions) -> bool;
        [[nodiscard]] static auto querySupportedApiVersion() -> std::uint32_t;

      public:
        /**
         * @brief Creates the engine's Vulkan instance and optional debug
         * messenger.
         *
         * Uses the supplied application name when populating Vulkan application
         * metadata during instance creation. Development diagnostics control
         * whether Vulkan validation support, debug-utils functionality, and the
         * instance-level debug messenger are enabled.
         *
         * @param application_name Name reported to Vulkan for the application.
         * @param development_diagnostics_enabled Whether development-oriented
         * Vulkan diagnostics should be enabled.
         *
         * @post Successful construction produces a non-null Vulkan instance
         * handle.
         * @post When development diagnostics are enabled, successful
         * construction produces a non-null Vulkan debug-messenger handle.
         *
         * @throws Core::Error::EngineError if required instance configuration,
         * extension discovery or validation, API-version discovery or
         * validation, Vulkan instance creation, or enabled diagnostic setup
         * fails.
         */
        explicit VulkanInstance(const std::string &application_name,
                                bool development_diagnostics_enabled);

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
