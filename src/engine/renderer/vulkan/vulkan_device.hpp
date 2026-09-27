#pragma once
#include "vulkan_device_features.hpp"
#include "vulkan_queue_requests.hpp"
#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Owns the engine's Vulkan logical device.
     *
     * Creates a logical device from a selected physical device using the
     * requested queue families, required device extensions, and explicitly
     * selected Vulkan features.
     *
     * Destruction releases the owned VkDevice. The physical device supplied
     * during construction is borrowed and is not owned by this object.
     *
     * The type is non-copyable because it exclusively owns a Vulkan device
     * handle.
     */
    class VulkanDevice {
      private:
        /** Vulkan logical-device handle owned by this object. */
        VkDevice m_Device{VK_NULL_HANDLE};

      public:
        /**
         * @brief Creates a Vulkan logical device.
         *
         * Creates one queue from each requested queue family, enables the
         * Vulkan device extensions required by Signum, and enables the supplied
         * core feature configuration.
         *
         * @param physical_device Physical device from which the logical device
         * is created.
         * @param queue_family_requests Unique queue families from which queues
         * are requested.
         * @param logical_device_configuration Core Vulkan features selected for
         * logical-device creation.
         *
         * @throws Core::Error::EngineError if Vulkan fails to create the
         * logical device.
         */
        VulkanDevice(
            VkPhysicalDevice physical_device,
            const std::vector<QueueFamilyRequest> &queue_family_requests,
            const LogicalDeviceFeatureConfiguration
                &logical_device_configuration);

        /**
         * @brief Destroys the owned Vulkan logical device.
         */
        ~VulkanDevice() noexcept;

        VulkanDevice(const VulkanDevice &) = delete;

        auto operator=(const VulkanDevice &) -> VulkanDevice & = delete;

        /**
         * @brief Returns the underlying Vulkan logical-device handle.
         *
         * The returned handle is non-owning and remains valid only while this
         * VulkanDevice object remains alive.
         *
         * @return Vulkan logical-device handle owned by this object.
         */
        [[nodiscard]] auto nativeHandle() const -> VkDevice;
    };
} // namespace SNE::Engine::Renderer::Vulkan
