#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Core::Logging {
    class Logger;
}

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
        /**
         * @brief Optional borrowed logger for Vulkan diagnostics.
         *
         * Receives Vulkan validation and diagnostic messages without
         * transferring ownership of the logging subsystem.
         *
         * A null pointer disables engine-backed diagnostic logging.
         *
         * @note A non-null logger must outlive this VulkanInstance and
         * all callbacks that access it.
         */
        Core::Logging::Logger *m_Logger{nullptr};

        /**
         * @brief Owned Vulkan instance handle.
         *
         * Establishes the Vulkan instance lifetime required by all
         * instance-dependent Vulkan resources.
         *
         * Initialized during instance creation and released after
         * the owned debug messenger has been destroyed.
         *
         * VK_NULL_HANDLE indicates that no instance has been created.
         */
        VkInstance m_Instance{VK_NULL_HANDLE};

        /**
         * @brief Optional owned Vulkan debug messenger.
         *
         * Receives validation and diagnostic messages when development
         * diagnostics are enabled.
         *
         * The messenger depends on m_Instance and must be destroyed
         * before the Vulkan instance.
         *
         * VK_NULL_HANDLE indicates that no messenger is owned.
         */
        VkDebugUtilsMessengerEXT m_DebugMessenger{VK_NULL_HANDLE};

        /**
         * @brief Validates configuration and creates the Vulkan instance.
         *
         * Checks the required Vulkan API version, instance extensions,
         * and optional validation-layer support before creating the
         * instance.
         *
         * When development diagnostics are enabled, attaches the
         * debug-messenger configuration to instance creation.
         *
         * @param application_name Name reported in Vulkan application metadata.
         * @param development_diagnostics_enabled Whether validation layers
         * and debug-utils support are required.
         *
         * @post On success, m_Instance contains a valid Vulkan instance handle.
         *
         * @throws Core::Error::EngineError if required Vulkan capabilities
         * are unavailable or instance creation fails.
         *
         * @note Does not create the persistent debug messenger.
         */
        auto createInstance(const std::string &application_name,
                            bool development_diagnostics_enabled) -> void;

        /**
         * @brief Checks whether the required Vulkan validation layer is
         * available.
         *
         * Enumerates supported instance layers and searches for the
         * engine's required validation layer.
         *
         * Repeats enumeration when Vulkan reports VK_INCOMPLETE.
         *
         * @return True when the required validation layer is available;
         * otherwise false.
         *
         * @throws Core::Error::EngineError if Vulkan layer enumeration fails.
         */
        [[nodiscard]] static auto checkValidationLayerSupport() -> bool;

        /**
         * @brief Collects the Vulkan instance extensions required by the
         * engine.
         *
         * Obtains the platform-required instance extensions from GLFW.
         * Includes VK_EXT_debug_utils when development diagnostics
         * are enabled.
         *
         * @param development_diagnostics_enabled Whether debug-utils
         * support must be included.
         *
         * @return Extension names required for Vulkan instance creation.
         *
         * @pre GLFW must be initialized.
         *
         * @throws Core::Error::EngineError if GLFW cannot provide the
         * required instance extensions.
         * @throws std::bad_alloc if extension storage allocation fails.
         *
         * @note The returned vector owns its pointer storage, but the
         * extension-name strings remain owned by their providers.
         */
        [[nodiscard]] static auto
        getRequiredExtensions(bool development_diagnostics_enabled)
            -> std::vector<const char *>;

        /**
         * @brief Creates the optional Vulkan debug messenger.
         *
         * Resolves the VK_EXT_debug_utils creation function and creates
         * a messenger using the configured callback, message severities,
         * message types, and borrowed diagnostic logger.
         *
         * @pre m_Instance must identify a valid Vulkan instance.
         * @pre VK_EXT_debug_utils must be enabled.
         *
         * @post On success, m_DebugMessenger contains a valid messenger handle.
         *
         * @throws Core::Error::EngineError if the creation function
         * is unavailable or messenger creation fails.
         *
         * @note The caller is responsible for releasing the Vulkan
         * instance if messenger initialization fails during construction.
         */
        auto setupDebugMessenger() -> void;

        /**
         * @brief Receives Vulkan validation and diagnostic messages.
         *
         * Processes callback information supplied by VK_EXT_debug_utils.
         * Returns VK_FALSE so the callback does not request abortion
         * of the Vulkan operation that produced the message.
         *
         * @param message_severity Severity assigned by Vulkan.
         * @param message_type Flags identifying the diagnostic message type.
         * @param callback_data Vulkan-provided diagnostic message information.
         * @param user_data Optional borrowed logger supplied through
         * the debug-messenger creation configuration.
         *
         * @return VK_FALSE to allow the Vulkan operation to continue.
         *
         * @pre callback_data must reference valid callback data.
         * @pre A non-null user_data pointer must identify a live logger.
         *
         * @note Exceptions must not propagate through this callback
         * into the Vulkan API.
         * @note Callback data is borrowed and must not be retained
         * beyond the invocation.
         */
        static VKAPI_ATTR VkBool32 VKAPI_CALL
        // NOLINTNEXTLINE(modernize-use-trailing-return-type)
        debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                      VkDebugUtilsMessageTypeFlagsEXT message_type,
                      const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                      void *user_data);

        /**
         * @brief Releases the owned Vulkan debug messenger when present.
         *
         * Resolves the instance-level destruction function and destroys
         * the messenger when the function is available.
         *
         * An absent messenger requires no cleanup.
         *
         * @post The stored debug-messenger handle is null.
         *
         * @note Does not destroy the Vulkan instance.
         */
        auto destroyDebugMessenger() noexcept -> void;

        /**
         * @brief Builds Vulkan debug-messenger callback configuration.
         *
         * Configures validation-message severity and type filtering,
         * the callback function, and its optional user data.
         *
         * The supplied logger is borrowed and passed through Vulkan's
         * callback user-data mechanism without transferring ownership.
         *
         * @param logger Optional logger made available to the callback.
         * @return Configured Vulkan debug-messenger creation information.
         *
         * @pre A non-null logger must remain alive for every callback
         * invocation using the returned configuration.
         *
         * @note The returned configuration does not own the logger.
         */
        [[nodiscard]] static auto
        makeDebugMessengerCreateInfo(Core::Logging::Logger *logger)
            -> VkDebugUtilsMessengerCreateInfoEXT;

        /**
         * @brief Checks whether all required Vulkan instance extensions exist.
         *
         * Enumerates available instance extensions and compares them
         * against the supplied requirements.
         *
         * Repeats enumeration when Vulkan reports VK_INCOMPLETE.
         *
         * @param required_extensions Borrowed names of extensions
         * required by the engine.
         *
         * @return True when every required extension is available;
         * otherwise false.
         *
         * @pre Every supplied extension name must reference a valid
         * null-terminated string.
         *
         * @throws Core::Error::EngineError if Vulkan extension
         * enumeration fails.
         */
        [[nodiscard]] static auto checkRequiredExtensionSupport(
            std::span<const char *const> required_extensions) -> bool;

        /**
         * @brief Queries the Vulkan loader's supported instance API version.
         *
         * Retrieves the API version reported by the Vulkan loader.
         * Compatibility with the engine's required API version is
         * validated separately during instance creation.
         *
         * @return Supported API version encoded using Vulkan's
         * version representation.
         *
         * @throws Core::Error::EngineError if the API version query fails.
         */
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
         * @param logger Optional borrowed logger for Vulkan diagnostics.
         * A null pointer disables engine-backed diagnostic logging.
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
                                bool development_diagnostics_enabled,
                                Core::Logging::Logger *logger = nullptr);

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
