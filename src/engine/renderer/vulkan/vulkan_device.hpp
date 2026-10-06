#pragma once
#include "vulkan_queue_requests.hpp"
#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    struct SelectedQueueFamilies;
    struct LogicalDeviceFeatureConfiguration;
    struct OptionalDeviceCapabilities;

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
         * Creates the queues described by the supplied queue-family requests,
         * enables the Vulkan device extensions required by Signum, and enables
         * the supplied logical-device feature configuration.
         *
         * Each queue-family request contains the priorities of the queues
         * requested from that family. The number of priorities therefore
         * determines the number of queues created from that family.
         *
         * @param physical_device Physical device from which the logical device
         * is created.
         * @param queue_families Selected graphics and presentation queue
         * families used to retrieve their corresponding logical-device queues.
         * @param queue_family_requests Unique queue-family requests describing
         * the queues to create and their priorities.
         * @param logical_device_configuration Vulkan features selected for
         * logical-device creation.
         * @param optional_capabilities Optional capabilities reported by the
         * selected physical device and used when selecting optional device
         * extensions to enable.
         *
         * @pre physical_device must be a valid Vulkan physical-device handle.
         * @pre queue_family_requests must contain requests for the selected
         * graphics and presentation queue families.
         * @pre Each queue-family request must use a unique queue-family index.
         * @pre Each queue-family request must contain at least one priority.
         * @pre Each queue-family request count must be representable by
         * std::uint32_t.
         * @pre Each queue priority must be within the range [0.0, 1.0].
         *
         * @post Successful construction owns a non-null Vulkan logical-device
         * handle.
         * @post Successful construction provides a valid graphics queue handle.
         * @post Successful construction provides a valid presentation queue
         * handle.
         *
         * @throws Core::Error::EngineError if Vulkan fails to create the
         * logical device.
         */
        VulkanDevice(
            VkPhysicalDevice physical_device,
            const SelectedQueueFamilies &queue_families,
            const std::vector<QueueFamilyRequest> &queue_family_requests,
            const LogicalDeviceFeatureConfiguration
                &logical_device_configuration,
            const OptionalDeviceCapabilities &optional_capabilities);

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
