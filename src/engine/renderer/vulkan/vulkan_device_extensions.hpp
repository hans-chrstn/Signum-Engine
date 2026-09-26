#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Queries the device extensions supported by a physical device.
     *
     * Retrieves the Vulkan device-extension properties reported for the
     * specified physical device.
     *
     * An empty result indicates that no device extensions were reported.
     *
     * @param device Physical device whose supported device extensions are
     * queried.
     *
     * @return Device-extension properties reported for the physical device.
     *
     * @throws Core::Error::EngineError if Vulkan fails to enumerate device
     *         extension properties.
     */
    [[nodiscard]] auto queryDeviceExtensionProperties(VkPhysicalDevice device)
        -> std::vector<VkExtensionProperties>;
} // namespace SNE::Engine::Renderer::Vulkan
