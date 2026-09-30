#include "application.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/vulkan/vulkan_command_pool.hpp"
#include "engine/renderer/vulkan/vulkan_device_discovery.hpp"
#include "engine/renderer/vulkan/vulkan_device_features.hpp"
#include "engine/renderer/vulkan/vulkan_device_selection.hpp"
#include "engine/renderer/vulkan/vulkan_queue_requests.hpp"
#include <cstdint>
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

    [[nodiscard]] auto requiredGraphicsQueueFamilyIndex(
        const Vulkan::PhysicalDeviceCandidate &candidate) -> std::uint32_t {
        const std::optional<std::uint32_t> &graphics_family =
            candidate.queue_family_indices.graphics_family;

        if (!graphics_family.has_value()) {
            SNE::Engine::Core::Assertion::failAssertion(
                SNE::Engine::Core::Assertion::AssertionType::Invariant,
                SNE::Engine::Core::Error::Subsystem::Vulkan,
                "Selected physical device requires a graphics queue family");
        }

        return graphics_family.value();
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
                      .presentModeFifoLatestReady == VK_TRUE),
          m_VulkanCommandPool(
              m_VulkanDevice.nativeHandle(),
              requiredGraphicsQueueFamilyIndex(m_PhysicalDeviceCandidate)) {}

    void Application::run() {
        while (!m_Window.shouldClose()) {
            Platform::Window::waitEvents();
        }
    }
} // namespace SNE::Engine::Core
