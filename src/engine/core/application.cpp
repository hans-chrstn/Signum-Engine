#include "application.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/renderer/vulkan/vulkan_device_discovery.hpp"
#include "engine/renderer/vulkan/vulkan_device_features.hpp"
#include "engine/renderer/vulkan/vulkan_device_selection.hpp"
#include "engine/renderer/vulkan/vulkan_queue_requests.hpp"
#include <optional>
#include <utility>
#include <vector>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;
namespace Error = SNE::Engine::Core::Error;

namespace {
    [[nodiscard]] auto selectRequiredPhysicalDevice(VkInstance instance,
                                                    VkSurfaceKHR surface)
        -> Vulkan::PhysicalDeviceCandidate {
        const std::vector<VkPhysicalDevice> available_devices =
            Vulkan::enumeratePhysicalDevices(instance);

        const std::vector<Vulkan::PhysicalDeviceCandidate> candidates =
            Vulkan::createPhysicalDeviceCandidates(available_devices, surface);

        std::optional<Vulkan::PhysicalDeviceCandidate> candidate =
            Vulkan::selectPhysicalDevice(candidates);

        if (!candidate.has_value()) {
            throw Error::EngineError(
                Error::Code::VulkanSuitablePhysicalDeviceUnavailable,
                "No suitable Vulkan physical device is available",
                "Obtain Vulkan Suitable Physical Device");
        }

        return std::move(candidate.value());
    }
} // namespace

namespace SNE::Engine::Core {
    Application::Application(ApplicationConfiguration configuration)
        : m_Configuration(std::move(configuration)),
          m_Window(m_Configuration.initial_window_size,
                   m_Configuration.application_name),
          m_VulkanInstance(m_Configuration.application_name,
                           m_Configuration.development_diagnostics_enabled),
          m_VulkanSurface(m_VulkanInstance.nativeHandle(),
                          m_Window.nativeHandle()),
          m_PhysicalDeviceCandidate(selectRequiredPhysicalDevice(
              m_VulkanInstance.nativeHandle(), m_VulkanSurface.nativeHandle())),
          m_LogicalDeviceFeatureConfiguration(
              Vulkan::deriveLogicalDeviceFeatureConfiguration(
                  Vulkan::deriveLogicalDeviceFeatureRequest(
                      m_Configuration.presentation_preference),
                  m_PhysicalDeviceCandidate.capabilities)),
          m_VulkanDevice(m_PhysicalDeviceCandidate.handle,
                         m_PhysicalDeviceCandidate.queue_family_indices,
                         Vulkan::deriveUniqueQueueFamilyRequests(
                             m_PhysicalDeviceCandidate.queue_family_indices),
                         m_LogicalDeviceFeatureConfiguration),
          m_VulkanSwapchain(
              m_PhysicalDeviceCandidate.handle, m_VulkanDevice.nativeHandle(),
              m_VulkanSurface.nativeHandle(),
              m_PhysicalDeviceCandidate.queue_family_indices,
              m_Window.framebufferSize(),
              m_Configuration.presentation_preference,
              m_LogicalDeviceFeatureConfiguration.fifo_latest_ready_feature
                      .presentModeFifoLatestReady == VK_TRUE) {}

    void Application::run() {
        while (!m_Window.shouldClose()) {
            Platform::Window::waitEvents();
        }
    }
} // namespace SNE::Engine::Core
