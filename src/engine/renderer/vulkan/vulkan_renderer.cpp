#include "vulkan_renderer.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/vulkan/vulkan_device_discovery.hpp"
#include "engine/renderer/vulkan/vulkan_device_features.hpp"
#include "engine/renderer/vulkan/vulkan_device_selection.hpp"
#include "engine/renderer/vulkan/vulkan_queue_requests.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;
namespace Error = SNE::Engine::Core::Error;

namespace {
    [[nodiscard]] auto selectRequiredPhysicalDevice(VkInstance instance,
                                                    VkSurfaceKHR surface)
        -> Vulkan::SelectedPhysicalDevice {
        const std::vector<VkPhysicalDevice> available_devices =
            Vulkan::enumeratePhysicalDevices(instance);

        const std::vector<Vulkan::DiscoveredPhysicalDevice> discovered_devices =
            Vulkan::inspectPhysicalDevices(available_devices, surface);

        std::optional<Vulkan::SelectedPhysicalDevice> selected_device =
            Vulkan::selectPhysicalDevice(discovered_devices);

        if (!selected_device.has_value()) {
            throw Error::EngineError(
                Error::Code::VulkanSuitablePhysicalDeviceUnavailable,
                "No suitable Vulkan physical device is available",
                "Obtain Vulkan Suitable Physical Device");
        }

        return selected_device.value();
    }

    constexpr std::size_t kFramesInFlight = 2;
} // namespace

namespace SNE::Engine::Renderer::Vulkan {
    VulkanRenderer::VulkanRenderer(
        const std::string &application_name, const Platform::Window &window,
        PresentationPreference presentation_preference,
        bool development_diagnostics_enabled)
        : m_Instance(application_name, development_diagnostics_enabled),
          m_Surface(m_Instance.nativeHandle(), window.nativeHandle()),
          m_PhysicalDevice(selectRequiredPhysicalDevice(
              m_Instance.nativeHandle(), m_Surface.nativeHandle())),
          m_LogicalDeviceFeatureConfiguration(
              deriveLogicalDeviceFeatureConfiguration(
                  deriveLogicalDeviceFeatureRequest(presentation_preference),
                  m_PhysicalDevice.capabilities)),
          m_Device(
              m_PhysicalDevice.handle, m_PhysicalDevice.queue_families,
              deriveUniqueQueueFamilyRequests(m_PhysicalDevice.queue_families),
              m_LogicalDeviceFeatureConfiguration),
          m_Swapchain(
              m_PhysicalDevice.handle, m_Device.nativeHandle(),
              m_Surface.nativeHandle(), m_PhysicalDevice.queue_families,
              window.framebufferSize(), presentation_preference,
              m_LogicalDeviceFeatureConfiguration.fifo_latest_ready_feature
                      .presentModeFifoLatestReady == VK_TRUE) {
        const std::uint32_t graphics_queue_family_index =
            m_PhysicalDevice.queue_families.graphics_family.family_index;

        m_FrameResources.reserve(kFramesInFlight);

        for (std::size_t i{}; i < kFramesInFlight; ++i) {
            m_FrameResources.emplace_back(m_Device.nativeHandle(),
                                          graphics_queue_family_index);
        }
    }
} // namespace SNE::Engine::Renderer::Vulkan
