#pragma once
#include "vulkan_queue_requests.hpp"
#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    struct QueueFamilyIndices;
    struct LogicalDeviceFeatureConfiguration;

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
        /** Non-owning graphics queue handle retrieved from the logical device.
         */
        VkQueue m_GraphicsQueue{VK_NULL_HANDLE};
        /** Non-owning presentation queue handle retrieved from the logical
         * device. */
        VkQueue m_PresentationQueue{VK_NULL_HANDLE};

      public:
        /**
         * @brief Creates a Vulkan logical device.
         *
         * Creates one queue from each requested queue family, enables the
         * Vulkan device extensions required by Signum, and enables the supplied
         * logical-device feature configuration.
         *
         * @param physical_device Physical device from which the logical device
         * is created.
         * @param queue_family_indices Graphics and presentation queue-family
         * indices used to retrieve their corresponding queues.
         * @param queue_family_requests Unique queue families from which queues
         * are requested.
         * @param logical_device_configuration Vulkan features selected for
         *                                     logical-device creation.
         *
         * @pre queue_family_indices contains both graphics and presentation
         *      queue-family indices.
         * @pre queue_family_requests contains requests for both the graphics
         * and presentation queue families identified by queue_family_indices.
         *
         * @throws Core::Error::EngineError if Vulkan fails to create the
         * logical device.
         */
        VulkanDevice(
            VkPhysicalDevice physical_device,
            const QueueFamilyIndices &queue_family_indices,
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
        [[nodiscard]] auto nativeHandle() const noexcept -> VkDevice;

        /**
         * @brief Returns the graphics queue associated with the logical device.
         *
         * The returned handle is non-owning and remains valid only while this
         * VulkanDevice object remains alive.
         *
         * @return Graphics queue retrieved from the logical device.
         */
        [[nodiscard]] auto graphicsQueue() const noexcept -> VkQueue;

        /**
         * @brief Returns the presentation queue associated with the logical
         * device.
         *
         * The returned handle is non-owning and remains valid only while this
         * VulkanDevice object remains alive.
         *
         * @return Presentation queue retrieved from the logical device.
         */
        [[nodiscard]] auto presentationQueue() const noexcept -> VkQueue;
    };
} // namespace SNE::Engine::Renderer::Vulkan
